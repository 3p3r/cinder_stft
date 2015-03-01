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
	ThreadRenderer(AudioNodes&, int frames_per_unit, int fft_size, int viewable_frames);

	void								update();
	void								draw();
	SpectralSurface&					getSurface(int index);
	ci::gl::Texture&					getTexture(int index);

private:
	std::vector<SpectralSurfaceRef>		mSurfacePool;
	std::vector<ci::gl::TextureRef>		mTexturePool;
	ci::gl::Fbo							mFramebuffer;
	int									mViewableFrames;
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_THREAD_RENDERER_H_