#ifndef CIEQ_INCLUDE_SCOPED_FBO_H_
#define CIEQ_INCLUDE_SCOPED_FBO_H_

#include <cinder/Area.h>
#include <cinder/gl/gl.h>

namespace cinder {
namespace gl {
class Fbo;
}} //!ci::gl

namespace cistft {

class ScopedFramebuffer
{
public:
	ScopedFramebuffer(cinder::gl::Fbo&);
	~ScopedFramebuffer();

private:
	cinder::gl::Fbo&				mFbo;
	ci::Area						mOriginalViewport;
	ci::gl::SaveFramebufferBinding	mSaveFb;
};

} // !namespace cistft

#endif // !CIEQ_INCLUDE_SCOPED_FBO_H_