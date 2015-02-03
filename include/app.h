#ifndef CIEQ_INCLUDE_APP_H_
#define CIEQ_INCLUDE_APP_H_

#include <cinder/app/AppNative.h>

namespace cieq
{

/*!
 * \class InputAnalyzer
 * \brief The main application class. Handles window initialization
 * window title, and other common juju.
 */
class InputAnalyzer final : public ci::app::AppNative
{
public:
	// \note Tells Cinder how to prepare the window
	void prepareSettings(Settings *settings) override final;

	void setup();
	void mouseDown(ci::app::MouseEvent event);
	void update();
	void draw();
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_H_