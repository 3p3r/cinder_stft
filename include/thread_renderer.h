#ifndef CIEQ_INCLUDE_THREAD_RENDERER_H_
#define CIEQ_INCLUDE_THREAD_RENDERER_H_

#include <vector>
#include <mutex>

#include "smart_surface.h"

#include <cinder/gl/Texture.h>
#include <cinder/gl/Fbo.h>

namespace cieq {
class AudioNodes;
class ThreadRenderer
{
public:
	ThreadRenderer(AudioNodes&, std::size_t frames_per_surface, std::size_t fft_size, std::size_t viewable_frames);

	void								update();
	void								draw();
	SpectralSurface&					getSurface(int index);
	ci::gl::Texture&					getTexture(int index);
	std::size_t							getViewableFrames() const;
	std::size_t							getFramesPerSurface() const;
	std::size_t							getFftSize() const;
	std::size_t							getSurfaceIndexByPos(std::size_t pos) const;
	std::size_t							getSurfaceInIndexByPos(std::size_t pos) const;

private:
	std::vector<SpectralSurfaceRef>		mSurfacePool;
	std::vector<ci::gl::TextureRef>		mTexturePool;
	std::mutex							mSurfaceLock;
	ci::gl::Fbo							mFramebuffer;
	std::size_t							mViewableFrames;
	std::size_t							mFramesPerSurface;
	std::size_t							mFftSize;
	std::size_t							mNumSurfaces;
	AudioNodes&							mAudioNodes;
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_THREAD_RENDERER_H_