#include "scoped_fbo.h"

#include <cinder/gl/Fbo.h>
#include <cinder/Camera.h>

namespace cieq {

ScopedFramebuffer::ScopedFramebuffer(cinder::gl::Fbo& fbo)
	: mFbo(fbo)
	, mOriginalViewport(ci::gl::getViewport())
{
	mFbo.bindFramebuffer();

	ci::gl::pushMatrices();
	ci::gl::setViewport(mFbo.getBounds());
	
	ci::CameraOrtho _camera;
	_camera.setOrtho(0, static_cast<float>(mFbo.getWidth()), static_cast<float>(mFbo.getHeight()), 0, 0.03f, 100.0f);
	ci::gl::setMatrices(_camera);
}

ScopedFramebuffer::~ScopedFramebuffer()
{
	ci::gl::popMatrices();
	ci::gl::setViewport(mOriginalViewport);
	
	mFbo.unbindFramebuffer();
}

}