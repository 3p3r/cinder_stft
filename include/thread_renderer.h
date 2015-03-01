#ifndef CIEQ_INCLUDE_THREAD_RENDERER_H_
#define CIEQ_INCLUDE_THREAD_RENDERER_H_

#include <vector>

#include "smart_surface.h"

#include <cinder/gl/Texture.h>
#include <cinder/gl/Fbo.h>

namespace cieq {
class AudioNodes;
class ThreadRenderer
{
public:
	ThreadRenderer(AudioNodes&);

	void								update();
	void								draw();
	SpectralSurface&					getSurface(int index);

private:
	std::vector<SpectralSurface>		mSurfacePool;
	std::vector<ci::gl::TextureRef>		mTexturePool;
	ci::gl::Fbo							mFramebuffer;
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_THREAD_RENDERER_H_