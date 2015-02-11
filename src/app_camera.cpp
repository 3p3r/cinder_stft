#include "app_camera.h"
#include "app_globals.h"
#include "app_event.h"

#include <cinder/app/App.h>

namespace cieq
{

AppCamera::AppCamera(AppGlobals& g)
	: mGlobals(g)
{}

void AppCamera::setup()
{
	mCameraPers.setEyePoint(ci::Vec3f(0.0f, 0.0f, 0.0f));
	mCameraPers.setCenterOfInterestPoint(ci::Vec3f(0.0f, 0.0f, 0.0f));
	mCameraPers.setPerspective(60.0f, ci::app::getWindowAspectRatio(), 1.0f, 1000.0f);
	mMayaCamera.setCurrentCam(mCameraPers);

	mGlobals.getEventProcessor().addMouseEvent([this](float x, float y) {
		processMouseDown(x, y);
	});
	
	mGlobals.getEventProcessor().addMouseDragEvent([this](float x, float y, bool l, bool m, bool r) {
		processMouseDrag(x, y, l, m, r);
	});
}

void AppCamera::resize()
{
	mCameraPers.setAspectRatio(ci::app::getWindowAspectRatio());
	mMayaCamera.setCurrentCam(mCameraPers);
}

void AppCamera::processMouseDown(float x, float y)
{
	mMayaCamera.mouseDown(ci::Vec2i(static_cast<int>(x), static_cast<int>(y)));
}

void AppCamera::processMouseDrag(float x, float y, bool l, bool m, bool r)
{
	mMayaCamera.mouseDrag(ci::Vec2i(static_cast<int>(x), static_cast<int>(y)), l, m, r);
}

ci::MayaCamUI& AppCamera::getMayaCamera()
{
	return mMayaCamera;
}

}