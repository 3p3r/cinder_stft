#ifndef CIEQ_INCLUDE_THREAD_RENDERER_H_
#define CIEQ_INCLUDE_THREAD_RENDERER_H_

#include <vector>
#include <array>
#include <mutex>

#include "smart_surface.h"

#include <cinder/gl/Texture.h>
#include <cinder/gl/Fbo.h>

namespace cieq {
class AppGlobals;

class ThreadRenderer
{
public:
	ThreadRenderer(AppGlobals&);

	void								update();
	void								setup();
	void								draw();
	SpectralSurface&					getSurface(int index, int pop_pos);
	std::size_t							getFramesPerSurface() const;
	std::size_t							getSurfaceIndexByQueryPos(std::size_t pos) const;
	std::size_t							getIndexInSurfaceByQueryPos(std::size_t pos) const;

private:
	using container_pair				= std::pair<SpectralSurfaceRef, ci::gl::TextureRef>;
	using container						= std::vector < container_pair >;

private:
	AppGlobals&							mGlobals;
	container							mSurfaceTexturePool;
	std::mutex							mPoolLock;
	std::size_t							mFramesPerSurface;
	std::size_t							mFftSize;
	std::size_t							mNumSurfaces;
	std::size_t							mTotalSurfacesLength;
	std::size_t							mLastSurfaceLength;
	std::size_t							mCacheTextureNum;
	std::atomic<int>					mLastPopPos;
	std::array<ci::gl::Fbo, 2>			mFrameBuffers;

private:
	std::size_t							calculateLastSurfaceLength() const;
	std::size_t							calculateTotalSurfacesLength() const;
	std::size_t							getActiveFramebuffer() const;
	void								drawFramebuffers(float shift_right = 0.0f, float shift_up = 0.0f);
	void								drawFramebuffer(ci::gl::Fbo& fbo, float shift_right = 0.0f, float shift_up = 0.0f);
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_THREAD_RENDERER_H_