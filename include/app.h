#ifndef CIEQ_INCLUDE_APP_H_
#define CIEQ_INCLUDE_APP_H_

#include <cinder/app/AppNative.h>
#include <cinder/params/Params.h>

#include "app_globals.h"
#include "app_event.h"
#include "audio_nodes.h"
#include "work_manager.h"
#include "app_config.h"
#include "stft_renderer.h"
#include "monitor_renderer.h"

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
	InputAnalyzer();

	//! tells Cinder how to prepare the window
	void			prepareSettings(Settings *settings) override final;
	//! sets up the application, this gets fired after Cinder is fully initialized.
	void			setup() override final;
	//! called at the beginning of every render loop, used to refresh data.
	void			update() override final;
	//! called at the end of every render loop, used to draw the data.
	void			draw() override final;
	//! application window resize callback.
	void			resize() override final;
	//! draws the current refresh rate of the render loop.
	void			drawFps();
	//! gets fired on mouse click
	void			mouseDown(ci::app::MouseEvent event) override final;
	//! gets fired on keyboard click
	void			keyDown(ci::app::KeyEvent event) override final;
	//! sets up the GUI
	void			setupGUI();

private:
	//! all user configurable parameters
	AppConfig		mAppConfig;
	//! the event processor class instance
	AppEvent		mEventProcessor;
	//! the work manager that accepts background works from main thread
	work::Manager	mWorkManager;
	//! the globals class instance, used to store common data
	AppGlobals		mGlobals;
	//! audio I/O class instance
	AudioNodes		mAudioNodes;
	//! STFT renderer
	StftRenderer	mStftRenderer;
	//! simple audio monitor renderer
	MonitorRenderer	mMonitorRenderer;
	//! ciUI instance
	ci::params::InterfaceGlRef
					mGuiInstance;
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_H_