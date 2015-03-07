#include "thread_renderer.h"
#include "app_globals.h"
#include "audio_nodes.h"
#include "recorder_node.h"
#include "scoped_fbo.h"

#include <cinder/app/App.h>

namespace cieq {

ThreadRenderer::ThreadRenderer(AppGlobals& globals)
	: mGlobals(globals)
	, mFramesPerSurface(mGlobals.getAudioNodes().getFormat().getSamplesCacheSize())
	, mFftSize(mGlobals.getAudioNodes().getFormat().getFftBins() / 2)
	, mLastPopPos(0)
	, mLastSurfaceLength(0)
	, mTotalSurfacesLength(0)
	, mCacheTextureNum(0)
{}

void ThreadRenderer::setup()
{
	if (!mGlobals.getAudioNodes().ready()) return;

	mNumSurfaces = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops() / mFramesPerSurface;
	if (mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops() % mFramesPerSurface != 0)
		mNumSurfaces += 1;

	const auto _time_span_percentage = mGlobals.getAudioNodes().getFormat().getTimeSpan() / mGlobals.getAudioNodes().getFormat().getRecordDuration();
	mNumSurfaces = static_cast<std::size_t>(_time_span_percentage * mNumSurfaces);

	mSurfaceTexturePool.resize(2 * mNumSurfaces);

	mLastSurfaceLength = calculateLastSurfaceLength();
	mTotalSurfacesLength = calculateTotalSurfacesLength();

	// I hate APIs with booleans in them :(
	for (auto index = 0; index < mFrameBuffers.size(); ++index)
	{
		mFrameBuffers[index] = ci::gl::Fbo(
			mFftSize, //width
			mTotalSurfacesLength, //height
			false, // alpha
			true, // color
			false); //depth
	}
}

void ThreadRenderer::update()
{
	if (!mGlobals.getAudioNodes().ready()) return;

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

			mCacheTextureNum++;
			pair.first.reset();
		}
	}
}

void ThreadRenderer::draw()
{
	if (!mGlobals.getAudioNodes().ready()) return;

	{ //enter FBO scope
		const auto a = getActiveFramebuffer();
		ScopedFramebuffer _scope(mFrameBuffers[getActiveFramebuffer()]);

		ci::gl::clear(ci::Color::white());

		for (int index = getActiveFramebuffer(); index < (getActiveFramebuffer() + 1) * mNumSurfaces; ++index)
		{
			ci::gl::pushMatrices();
			ci::gl::translate(0.0f, index * static_cast<float>(getFramesPerSurface()));

			if (mSurfaceTexturePool[index].first)
			{
				// draw surface
				ci::gl::draw(*mSurfaceTexturePool[index].first);
			}
			else if (mSurfaceTexturePool[index].second)
			{
				// draw texture
				ci::gl::draw(mSurfaceTexturePool[index].second);
			}

			ci::gl::popMatrices();
		}
	}

	drawFramebuffers();
}

SpectralSurface& ThreadRenderer::getSurface(int index, int pop_pos)
{
	const auto _moded_index = index % (2 * mNumSurfaces);
	// if surface does not exist
	if (!mSurfaceTexturePool[_moded_index].first)
	{
		// lock and construct the surface
		std::lock_guard<std::mutex> _lock(mPoolLock);
		if (!mSurfaceTexturePool[_moded_index].first) //double check
		{
			if (_moded_index != mNumSurfaces - 1 || _moded_index != 2 * (mNumSurfaces - 1))
			{
				mSurfaceTexturePool[_moded_index].first = std::make_unique<SpectralSurface>(mFftSize, mLastSurfaceLength);
			}
			else
			{
				mSurfaceTexturePool[_moded_index].first = std::make_unique<SpectralSurface>(mFftSize, getFramesPerSurface());
			}
		}
	}
	
	mLastPopPos = pop_pos; //no lock needed, atomic
	return *(mSurfaceTexturePool[_moded_index].first);
}

std::size_t ThreadRenderer::getFramesPerSurface() const
{
	return mFramesPerSurface;
}

std::size_t ThreadRenderer::getSurfaceIndexByQueryPos(std::size_t pos) const
{
	const auto pop_index = mGlobals.getAudioNodes().getBufferRecorderNode()->getQueryIndexByQueryPos(pos);
	return pop_index / getFramesPerSurface();
}

std::size_t ThreadRenderer::getIndexInSurfaceByQueryPos(std::size_t pos) const
{
	const auto pop_index = mGlobals.getAudioNodes().getBufferRecorderNode()->getQueryIndexByQueryPos(pos);
	return pop_index % getFramesPerSurface();
}

std::size_t ThreadRenderer::calculateLastSurfaceLength() const
{
	const auto _time_span_percentage = mGlobals.getAudioNodes().getFormat().getTimeSpan() / mGlobals.getAudioNodes().getFormat().getRecordDuration();
	const auto _max_pops = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops();
	const auto _pops_per_fbo = static_cast<std::size_t>(_time_span_percentage * _max_pops);
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _pops_per_fbo;
	return getFramesPerSurface() - _actual_minus_real_diff;
}

std::size_t ThreadRenderer::calculateTotalSurfacesLength() const
{
	const auto _time_span_percentage = mGlobals.getAudioNodes().getFormat().getTimeSpan() / mGlobals.getAudioNodes().getFormat().getRecordDuration();
	const auto _max_pops = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops();
	const auto _pops_per_fbo = static_cast<std::size_t>(_time_span_percentage * _max_pops);
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _pops_per_fbo;
	return (mNumSurfaces * mFramesPerSurface) - _actual_minus_real_diff;
}

void ThreadRenderer::drawFramebuffer(ci::gl::Fbo& fbo, float shift_right /*= 0.0f*/, float shift_up /*= 0.0f*/)
{
	ci::gl::pushMatrices();

	ci::gl::translate(ci::app::getWindowWidth() + shift_right, ci::app::getWindowHeight() + shift_up);
	ci::gl::rotate(ci::Vec3f(180.0f, 0, 90.0f));
	ci::gl::scale(
		static_cast<float>(ci::app::getWindowHeight()) / fbo.getWidth(),
		static_cast<float>(ci::app::getWindowWidth()) / fbo.getHeight());
	ci::gl::draw(fbo.getTexture());

	ci::gl::popMatrices();
}

void ThreadRenderer::drawFramebuffers(float shift_right /*= 0.0f*/, float shift_up /*= 0.0f*/)
{
	drawFramebuffer(mFrameBuffers[0], shift_right);
	drawFramebuffer(mFrameBuffers[1], shift_right + ci::app::getWindowWidth());
}

std::size_t ThreadRenderer::getActiveFramebuffer() const
{
	return 0; // mCacheTextureNum / mNumSurfaces;
}

} //!cieq