#ifndef CIEQ_INCLUDE_APP_CAMERA_H_
#define CIEQ_INCLUDE_APP_CAMERA_H_

#include <cinder/MayaCamUI.h>

namespace cieq {
class AppGlobals;

class AppCamera
{
public:
	AppCamera(AppGlobals& g);
	void			setup();
	void			resize();

	ci::MayaCamUI&	getMayaCamera();

private:
	void			processMouseDown(float, float);
	void			processMouseDrag(float, float, bool, bool, bool);

private:
	AppGlobals&		mGlobals;
	ci::CameraPersp	mCameraPers;
	ci::MayaCamUI	mMayaCamera;
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_APP_CAMERA_H_