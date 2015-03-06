#include "thread_renderer.h"
#include "audio_nodes.h"
#include "recorder_node.h"

#include <cinder/app/App.h>

namespace cieq {

ThreadRenderer::ThreadRenderer(AudioNodes& nodes)
	: mFramesPerSurface(nodes.getFormat().getSamplesCacheSize())
	, mFftSize(nodes.getFormat().getFftBins() / 2)
	, mAudioNodes(nodes)
	, mLastPopPos(0)
	, mLastSurfaceLength(0)
	, mTotalSurfacesLength(0)
{}

void ThreadRenderer::setup()
{
	if (!mAudioNodes.ready()) return;

	mNumSurfaces = mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops() / mFramesPerSurface;
	if (mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops() % mFramesPerSurface != 0)
		mNumSurfaces += 1;

	mSurfaceTexturePool.resize(mNumSurfaces);

	mLastSurfaceLength = calculateLastSurfaceLength();
	mTotalSurfacesLength = calculateTotalSurfacesLength();

	// I hate APIs with booleans in them :(
	mCompleteAudioFbo = ci::gl::Fbo(
		mTotalSurfacesLength, //width
		mFftSize, //height
		false, // alpha
		true, // color
		false); //depth
}

void ThreadRenderer::update()
{
	if (!mAudioNodes.ready()) return;

	for (container_pair& pair : mSurfaceTexturePool)
	{
		if (pair.first && pair.first->allRowsTouched())
		{
			if (pair.second)
			{
				pair.second->update(*pair.first);
			}
			else
			{
				pair.second = ci::gl::Texture::create(*pair.first);
				pair.second->setMinFilter(GL_NEAREST); //disable GPU blur
				pair.second->setMagFilter(GL_NEAREST); //disable GPU blur
			}

			pair.first.reset();
		}
	}
}

void ThreadRenderer::draw()
{
	if (!mAudioNodes.ready()) return;

	{ //enter FBO scope
		ci::gl::SaveFramebufferBinding _save_fb;
		mCompleteAudioFbo.bindFramebuffer();

		ci::gl::pushMatrices();
		ci::gl::clear();
		ci::gl::rotate(90.0f); //rotate scene 90 degrees

		// just a convenience
		auto _recorder_node = mAudioNodes.getBufferRecorderNode();

		// get current record position
		const int _current_write_pos = mLastPopPos;
		// percentage of the entire audio done recording
		const float _percentage_done = static_cast<float>(_current_write_pos) / _recorder_node->getNumFrames();
		// get the index of last active surface for drawing
		int _current_last_surface = static_cast<int>(_percentage_done * mNumSurfaces);
		// number of samples that will be empty drawn (last surface)
		// get last thread-reported pop position and divide it over max pops possible
		const float _current_index_in_surface = static_cast<float>(getIndexInSurfaceByQueryPos(mLastPopPos));
		// shift to left for OpenGL (we're moving textures upside-down, therefore we shift one entire surface to right + estimate index in surface + static offset)
		const float _shift_right = static_cast<float>(_current_index_in_surface - mFramesPerSurface) - ci::app::getWindowWidth();

		ci::gl::translate(0.0f, _shift_right); //after rotation, moving x is like moving y

		for (int index = _current_last_surface, count = 0; index >= 0; --index, ++count)
		{
			const auto t = ci::app::getWindowHeight() / mCompleteAudioFbo.getHeight();
			const auto x1 = static_cast<float>(mFftSize) * ci::app::getWindowHeight() / mCompleteAudioFbo.getHeight();
			const auto y1 = static_cast<float>(count + 1) * mFramesPerSurface;
			const auto x2 = 0.0f;
			const auto y2 = static_cast<float>(count)* mFramesPerSurface;

			ci::Rectf draw_rect(x1, y1, x2, y2);

			if (mSurfaceTexturePool[index].first)
			{
				// draw surface
				ci::gl::draw(*mSurfaceTexturePool[index].first, draw_rect);
			}
			else if (mSurfaceTexturePool[index].second)
			{
				// draw texture
				ci::gl::draw(mSurfaceTexturePool[index].second, draw_rect);
			}
		}

		ci::gl::popMatrices();

		mCompleteAudioFbo.unbindFramebuffer();
	}

	{// enter screen drawing scope
		ci::gl::pushMatrices();
		
		ci::Rectf _target_rect(mCompleteAudioFbo.getBounds());
		std::swap(_target_rect.y1, _target_rect.y2); //swap UVs
		ci::gl::draw(mCompleteAudioFbo.getTexture(), _target_rect);
		
		ci::gl::popMatrices();
	}
}

SpectralSurface& ThreadRenderer::getSurface(int index, int pop_pos)
{
	// if surface does not exist
	if (!mSurfaceTexturePool[index].first)
	{
		// lock and construct the surface
		std::lock_guard<std::mutex> _lock(mPoolLock);
		if (!mSurfaceTexturePool[index].first) //double check
		{
			if (index != mNumSurfaces - 1)
			{
				mSurfaceTexturePool[index].first = std::make_unique<SpectralSurface>(mFftSize, mLastSurfaceLength);
			}
			else
			{
				mSurfaceTexturePool[index].first = std::make_unique<SpectralSurface>(mFftSize, getFramesPerSurface());
			}
		}
	}
	
	mLastPopPos = pop_pos; //no lock needed, atomic
	return *(mSurfaceTexturePool[index].first);
}

std::size_t ThreadRenderer::getFramesPerSurface() const
{
	return mFramesPerSurface;
}

std::size_t ThreadRenderer::getSurfaceIndexByQueryPos(std::size_t pos) const
{
	const auto pop_index = mAudioNodes.getBufferRecorderNode()->getQueryIndexByQueryPos(pos);
	return pop_index / getFramesPerSurface();
}

std::size_t ThreadRenderer::getIndexInSurfaceByQueryPos(std::size_t pos) const
{
	const auto pop_index = mAudioNodes.getBufferRecorderNode()->getQueryIndexByQueryPos(pos);
	return pop_index % getFramesPerSurface();
}

void ThreadRenderer::cleanSurfaces()
{
	for (container_pair& pair : mSurfaceTexturePool)
	{
		if (pair.first)
		{
			pair.first.reset();
		}
	}
}

std::size_t ThreadRenderer::calculateLastSurfaceLength() const
{
	const auto _max_pops = mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops();
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _max_pops;
	return getFramesPerSurface() - _actual_minus_real_diff;
}

std::size_t ThreadRenderer::calculateTotalSurfacesLength() const
{
	const auto _max_pops = mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops();
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _max_pops;
	return (mNumSurfaces * mFramesPerSurface) - _actual_minus_real_diff;
}

} //!cieq