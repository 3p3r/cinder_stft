#include "thread_renderer.h"
#include "audio_nodes.h"
#include "recorder_node.h"

namespace cieq {

ThreadRenderer::ThreadRenderer(AudioNodes& nodes, std::size_t frames_per_surface, std::size_t fft_size, std::size_t viewable_frames)
	: mViewableFrames(viewable_frames)
	, mFramesPerSurface(frames_per_surface)
	, mFftSize(fft_size)
{
	mFramebuffer = ci::gl::Fbo(mViewableFrames, mFftSize);
}

void ThreadRenderer::update()
{
	// change surfaces to textures here
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

} //!cieq