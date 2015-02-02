#ifndef CIEQ_INCLUDE_APP_H_
#define CIEQ_INCLUDE_APP_H_

#include <cinder/app/AppNative.h>

namespace cieq
{

class InputAnalyzer : public ci::app::AppNative
{
public:
	void setup();
	void mouseDown(ci::app::MouseEvent event);
	void update();
	void draw();
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_H_