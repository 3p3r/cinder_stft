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
{}

void ThreadRenderer::setup()
{
	if (!mGlobals.getAudioNodes().ready()) return;

	mNumSurfaces = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops() / mFramesPerSurface;
	if (mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops() % mFramesPerSurface != 0)
		mNumSurfaces += 1;

	mSurfaceTexturePool.resize(mNumSurfaces);

	mLastSurfaceLength = calculateLastSurfaceLength();
	mTotalSurfacesLength = calculateTotalSurfacesLength();

	// I hate APIs with booleans in them :(
	mCompleteAudioFbo = ci::gl::Fbo(
		mFftSize, //width
		mTotalSurfacesLength, //height
		false, // alpha
		true, // color
		false); //depth
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

			pair.first.reset();
		}
	}
}

void ThreadRenderer::draw()
{
	if (!mGlobals.getAudioNodes().ready()) return;

	{ //enter FBO scope
		ScopedFramebuffer _scope(mCompleteAudioFbo);

		ci::gl::clear(ci::Color::white());

		for (int index = 0; index < mNumSurfaces; ++index)
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

	{// enter screen drawing scope
		ci::gl::pushMatrices();

		ci::gl::translate(ci::app::getWindowWidth(), 0.0f);
		ci::gl::rotate(90.0f);
		ci::gl::scale(
			(float)ci::app::getWindowHeight() / mCompleteAudioFbo.getWidth(),
			(float)ci::app::getWindowWidth() / mCompleteAudioFbo.getHeight());
		ci::gl::draw(mCompleteAudioFbo.getTexture());
		
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
	const auto _max_pops = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops();
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _max_pops;
	return getFramesPerSurface() - _actual_minus_real_diff;
}

std::size_t ThreadRenderer::calculateTotalSurfacesLength() const
{
	const auto _max_pops = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops();
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _max_pops;
	return (mNumSurfaces * mFramesPerSurface) - _actual_minus_real_diff;
}

} //!cieq