#include "thread_renderer.h"
#include "audio_nodes.h"
#include "recorder_node.h"

namespace cieq {

ThreadRenderer::ThreadRenderer(AudioNodes& nodes, std::size_t frames_per_surface, std::size_t fft_size, std::size_t viewable_frames)
	: mViewableFrames(viewable_frames)
	, mFramesPerSurface(frames_per_surface)
	, mFftSize(fft_size)
	, mAudioNodes(nodes)
{
	mNumSurfaces = mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops() / mFramesPerSurface;
	if (mAudioNodes.getBufferRecorderNode()->getMaxPossiblePops() % mFramesPerSurface != 0)
		mNumSurfaces += 1;

	mSurfacePool.resize(mNumSurfaces);
	mTexturePool.resize(mNumSurfaces);

	mFramebuffer = ci::gl::Fbo(mViewableFrames, mFftSize);
}

void ThreadRenderer::update()
{
	int index = 0;
	for (SpectralSurfaceRef& surface : mSurfacePool)
	{
		if (surface && surface->allColsTouched())
		{
			mTexturePool[index] = ci::gl::Texture::create(*surface);
			surface.reset(); //clear surface
		}
		index++;
	}
}

void ThreadRenderer::draw()
{
	ci::gl::SaveFramebufferBinding _bindings;
	
	//! draw everything to a frame buffer
	mFramebuffer.bindFramebuffer();

	// draw here

	mFramebuffer.unbindFramebuffer();
}

SpectralSurface& ThreadRenderer::getSurface(int index)
{
	if (!mSurfacePool[index])
	{
		std::lock_guard<std::mutex> _lock(mSurfaceLock);
		if (!mSurfacePool[index]) //double check
		{
			mSurfacePool[index] = std::make_unique<SpectralSurface>(getFramesPerSurface(), getFftSize());
		}
	}
	return *(mSurfacePool[index]);
}

ci::gl::Texture& ThreadRenderer::getTexture(int index)
{
	return *mTexturePool[index].get();
}

std::size_t ThreadRenderer::getViewableFrames() const
{
	return mViewableFrames;
}

std::size_t ThreadRenderer::getFramesPerSurface() const
{
	return mFramesPerSurface;
}

std::size_t ThreadRenderer::getFftSize() const
{
	return mFftSize;
}

std::size_t ThreadRenderer::getSurfaceIndexByPos(std::size_t pos) const
{
	const auto pop_index = mAudioNodes.getBufferRecorderNode()->getQueryIndexByPos(pos);
	return pop_index / getFramesPerSurface();
}

std::size_t ThreadRenderer::getSurfaceInIndexByPos(std::size_t pos) const
{
	const auto pop_index = mAudioNodes.getBufferRecorderNode()->getQueryIndexByPos(pos);
	return pop_index % getFramesPerSurface();
}

} //!cieq