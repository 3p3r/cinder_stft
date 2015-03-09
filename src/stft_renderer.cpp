#include "stft_renderer.h"
#include "app_globals.h"
#include "audio_nodes.h"
#include "recorder_node.h"
#include "scoped_fbo.h"
#include "stft_filter.h"

#include <cinder/app/App.h>

namespace cieq {

StftRenderer::StftRenderer(AppGlobals& globals)
	: mGlobals(globals)
	, mFramesPerSurface(mGlobals.getAudioNodes().getFormat().getSamplesCacheSize())
	, mViewableBins(mGlobals.getFilter().getViewableBins())
	, mLastPopPos(0)
	, mLastSurfaceLength(0)
	, mTotalSurfacesLength(0)
{}

void StftRenderer::setup()
{
	if (!mGlobals.getAudioNodes().isRecorderReady()) return;

	mNumSurfaces = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops() / mFramesPerSurface;
	if (mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops() % mFramesPerSurface != 0)
		mNumSurfaces += 1;

	const auto _time_span_percentage = mGlobals.getAudioNodes().getFormat().getTimeSpan() / mGlobals.getAudioNodes().getFormat().getRecordDuration();
	mNumSurfaces = static_cast<std::size_t>(_time_span_percentage * mNumSurfaces);

	mSurfaceTexturePool.resize(2 * mNumSurfaces);

	mLastSurfaceLength = calculateLastSurfaceLength();
	mTotalSurfacesLength = calculateTotalSurfacesLength();

	// I hate APIs with booleans in them :(
	for (std::size_t index = 0; index < mFrameBuffers.size(); ++index)
	{
		mFrameBuffers[index] = ci::gl::Fbo(
			mViewableBins, //width
			mTotalSurfacesLength, //height
			false, // alpha
			true, // color
			false); //depth

		mFrameBuffers[index].getTexture().setMinFilter(GL_NEAREST); //disable GPU blur
		mFrameBuffers[index].getTexture().setMagFilter(GL_NEAREST); //disable GPU blur
	}
}

void StftRenderer::update()
{
	if (!mGlobals.getAudioNodes().isRecorderReady()) return;

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

void StftRenderer::draw()
{
	if (!mGlobals.getAudioNodes().isRecorderReady()) return;

	{ //enter FBO scope
		const auto _active_fbo = getActiveFramebuffer();
		ScopedFramebuffer _scope(mFrameBuffers[_active_fbo]);

		ci::gl::clear();

		for (std::size_t index = _active_fbo * mNumSurfaces; index < (_active_fbo + 1) * mNumSurfaces; ++index)
		{
			ci::gl::pushMatrices();
			ci::gl::translate(0.0f, (index % mNumSurfaces) * static_cast<float>(getFramesPerSurface()));

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

	drawFramebuffers(calculateActiveFboOffset());
}

StftSurface& StftRenderer::getSurface(int index, int pop_pos)
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
				mSurfaceTexturePool[_moded_index].first = std::make_unique<StftSurface>(mViewableBins, mLastSurfaceLength, mGlobals.getFilter().getMagnitudeIndexStart());
			}
			else
			{
				mSurfaceTexturePool[_moded_index].first = std::make_unique<StftSurface>(mViewableBins, getFramesPerSurface(), mGlobals.getFilter().getMagnitudeIndexStart());
			}
		}
	}
	
	mLastPopPos = pop_pos; //no lock needed, atomic
	return *(mSurfaceTexturePool[_moded_index].first);
}

std::size_t StftRenderer::getFramesPerSurface() const
{
	return mFramesPerSurface;
}

std::size_t StftRenderer::getSurfaceIndexByQueryPos(std::size_t pos) const
{
	const auto pop_index = mGlobals.getAudioNodes().getBufferRecorderNode()->getQueryIndexByQueryPos(pos);
	return (pop_index / getFramesPerSurface()) % (2 * mNumSurfaces);
}

std::size_t StftRenderer::getIndexInSurfaceByQueryPos(std::size_t pos) const
{
	const auto pop_index = mGlobals.getAudioNodes().getBufferRecorderNode()->getQueryIndexByQueryPos(pos);
	return pop_index % getFramesPerSurface();
}

std::size_t StftRenderer::calculateLastSurfaceLength() const
{
	const auto _time_span_percentage = mGlobals.getAudioNodes().getFormat().getTimeSpan() / mGlobals.getAudioNodes().getFormat().getRecordDuration();
	const auto _max_pops = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops();
	const auto _pops_per_fbo = static_cast<std::size_t>(_time_span_percentage * _max_pops);
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _pops_per_fbo;
	return getFramesPerSurface() - _actual_minus_real_diff;
}

std::size_t StftRenderer::calculateTotalSurfacesLength() const
{
	const auto _time_span_percentage = mGlobals.getAudioNodes().getFormat().getTimeSpan() / mGlobals.getAudioNodes().getFormat().getRecordDuration();
	const auto _max_pops = mGlobals.getAudioNodes().getBufferRecorderNode()->getMaxPossiblePops();
	const auto _pops_per_fbo = static_cast<std::size_t>(_time_span_percentage * _max_pops);
	const auto _max_num_pops_in_surfaces = mNumSurfaces * mFramesPerSurface;
	const auto _actual_minus_real_diff = _max_num_pops_in_surfaces - _pops_per_fbo;
	return (mNumSurfaces * mFramesPerSurface) - _actual_minus_real_diff;
}

void StftRenderer::drawFramebuffer(ci::gl::Fbo& fbo, float shift_right /*= 0.0f*/, float shift_up /*= 0.0f*/)
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

void StftRenderer::drawFramebuffers(float shift_right /*= 0.0f*/, float shift_up /*= 0.0f*/)
{	
	const auto _active_fbo = getActiveFramebuffer();
	const auto _inactive_fbo = (_active_fbo == 1 ? 0 : 1);

	drawFramebuffer(mFrameBuffers[_active_fbo], shift_right);
	drawFramebuffer(mFrameBuffers[_inactive_fbo], shift_right - ci::app::getWindowWidth());
}

std::size_t StftRenderer::getActiveFramebuffer() const
{
	return (getSurfaceIndexByQueryPos(mLastPopPos) / mNumSurfaces);
}

float StftRenderer::calculateActiveFboOffset() const
{
	// get current record position (copying the atomic to be safe)
	const int _current_write_pos = mLastPopPos;
	// get the index of last active surface for drawing
	int _current_surface_index = getSurfaceIndexByQueryPos(_current_write_pos);
	if (_current_surface_index >= static_cast<int>(mNumSurfaces))
	{
		_current_surface_index = _current_surface_index % mNumSurfaces;
	}
	// number of samples that will be empty drawn (last surface)
	// get last thread-reported pop position and divide it over max pops possible
	const int _current_index_in_surface = getIndexInSurfaceByQueryPos(_current_write_pos);
	// How far thread manager has filled this surface approximately?
	const int _filled_length = _current_surface_index * mFramesPerSurface + _current_index_in_surface;
	// How much more is left?
	const int _unfilled_length = mTotalSurfacesLength - _filled_length;
	// Scale factor to window
	const float _window_to_fbo_scale = static_cast<float>(ci::app::getWindowWidth()) / mTotalSurfacesLength;
	
	return _window_to_fbo_scale * _unfilled_length;
}

} //!cieq