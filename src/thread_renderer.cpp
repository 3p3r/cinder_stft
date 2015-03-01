#include "thread_renderer.h"
#include "audio_nodes.h"
#include "recorder_node.h"

namespace cieq {

ThreadRenderer::ThreadRenderer(AudioNodes& nodes, int frames_per_unit, int fft_size, int viewable_frames)
	: mViewableFrames(viewable_frames)
{
	int num_blocks = (nodes.getBufferRecorderNode()->getNumFrames() / frames_per_unit) + 1;
	mSurfacePool.resize(num_blocks);
	mTexturePool.resize(num_blocks);
	for (auto index = 0; index < num_blocks; ++index)
	{
		mSurfacePool.push_back(std::make_unique<SpectralSurface>(frames_per_unit, fft_size));
		mTexturePool.push_back(ci::gl::Texture::create(*mSurfacePool.back()));
	}

	mFramebuffer = ci::gl::Fbo(viewable_frames, fft_size);
}

void ThreadRenderer::update()
{
	auto index = 0;
	for (SpectralSurfaceRef& surface : mSurfacePool)
	{
		if (*surface && surface->allRowsTouched())
		{
			mTexturePool[index]->update(*surface);
			surface->reset();
		}
		++index;
	}
}

void ThreadRenderer::draw()
{
	auto index = 0;

	ci::gl::SaveFramebufferBinding _bindings;
	
	//! draw everything to a frame buffer
	mFramebuffer.bindFramebuffer();

	for (SpectralSurfaceRef& surface : mSurfacePool)
	{
		if (*surface)
		{
			//draw surface here.
		}
		else
		{
			//draw mTexturePool[index] here.
		}
		++index;
	}

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

} //!cieq