#include "thread_renderer.h"
#include "audio_nodes.h"
#include "recorder_node.h"

#include <cinder/app/App.h>

namespace cieq {

ThreadRenderer::ThreadRenderer(AudioNodes& nodes, std::size_t frames_per_surface, std::size_t fft_size)
	: mFramesPerSurface(frames_per_surface)
	, mFftSize(fft_size)
	, mAudioNodes(nodes)
{
	mNumSurfaces = mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops() / mFramesPerSurface;
	if (mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops() % mFramesPerSurface != 0)
		mNumSurfaces += 1;

	mSurfaceTexturePool.resize(mNumSurfaces);
}

void ThreadRenderer::update()
{
	for (container_pair& pair : mSurfaceTexturePool)
	{
		if (pair.first && pair.first->allRowsTouched())
		{
			pair.second = ci::gl::Texture::create(*pair.first);
			pair.first.reset();
		}
	}
}

void ThreadRenderer::draw()
{
	ci::gl::pushMatrices();
	ci::gl::clear(ci::Color::black());
	ci::gl::rotate(90.0f); //rotate scene 90 degrees
	ci::gl::scale(1.0f, 1.0f);

	auto _recorder_node = mAudioNodes.getBufferRecorderNode();
	const int _viewable_surfaces = getCurrentViewableSurfaces();
	int _current_write_pos = _recorder_node->getWritePosition();
	if (!_recorder_node->canQuery()) _current_write_pos -= 1; //end of samples
	const float _percentage_done = static_cast<float>(_current_write_pos) / _recorder_node->getNumFrames();
	const float _percentage_full = static_cast<float>(_recorder_node->getNumFrames() - 1) / _recorder_node->getNumFrames();
	int _current_last_surface = static_cast<int>(_percentage_done * mNumSurfaces);
	if (_current_last_surface == mNumSurfaces) _current_last_surface -= 1; //fix me
	const float _static_offset = std::fmodf(_percentage_full * mNumSurfaces * mFramesPerSurface, mFramesPerSurface);
	const float _current_index_in_surface = std::fmodf(_percentage_done * mNumSurfaces * mFramesPerSurface, mFramesPerSurface);

	ci::gl::translate(0.0f, (_current_index_in_surface - mFramesPerSurface / 2 - _static_offset) - ci::app::getWindowWidth()); //after rotation, moving x is like moving y

	for (int index = _current_last_surface, count = 0; count <= _viewable_surfaces && index >= 0; --index, ++count)
	{
		ci::Rectf draw_rect(mFftSize, (count + 1) * mFramesPerSurface, 0, count * mFramesPerSurface);

		if (mSurfaceTexturePool[index].second)
		{
			// draw texture
			ci::gl::draw(mSurfaceTexturePool[index].second, draw_rect);
		}
		else if (mSurfaceTexturePool[index].first)
		{
			// draw surface
			ci::gl::draw(*mSurfaceTexturePool[index].first, draw_rect);
		}
	}

	ci::gl::popMatrices();
}

SpectralSurface& ThreadRenderer::getSurface(int index)
{
	// if surface does not exist
	if (!mSurfaceTexturePool[index].first)
	{
		// lock and construct the surface
		std::lock_guard<std::mutex> _lock(mPoolLock);
		if (!mSurfaceTexturePool[index].first) //double check
		{
			mSurfaceTexturePool[index].first = std::make_unique<SpectralSurface>(mFftSize, getFramesPerSurface());
		}
	}

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

std::size_t ThreadRenderer::getCurrentViewableSurfaces() const
{
	return (ci::app::getWindowWidth() / mFramesPerSurface) + ((ci::app::getWindowWidth() % mFramesPerSurface == 0) ? 0 : 1);
}

} //!cieq