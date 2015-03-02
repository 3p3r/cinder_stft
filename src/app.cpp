#include "app.h"

#include <cinder/params/Params.h>

namespace cieq
{

InputAnalyzer::InputAnalyzer()
	: mGlobals(mEventProcessor, mWorkManager, mAudioNodes)
	, mAudioNodes(mGlobals)
{}

void InputAnalyzer::prepareSettings(Settings *settings)
{
	// Enables the console window to show up beside the graphical window
	// We need the console for debugging and logging purposes
	settings->enableConsoleWindow(true);

	// It's ok if a user resizes the window, we'll respond accordingly
	settings->setResizable(true);

	// Title of the graphical window when it pops up
	settings->setTitle("Cinder Audio Equalizer");

	// Get the current display (monitor) dimensions
	const auto current_display_size = settings->getDisplay()->getSize();
	// calculate the window size to be 90% of the monitor size
	const auto window_size = (current_display_size * 9) / 10;
	// calculate the window position (top left) to be at 10% of monitor size
	const auto window_position = (current_display_size * 5) / 100;

	// set the window size to be 90% of the monitor size
	settings->setWindowSize(window_size);
	// set the window position (top left) to be at 10% of monitor size
	settings->setWindowPos(window_position);
}

void InputAnalyzer::setup()
{
	// setup the interface GL params
	setupParamsGl();
	// setup audio I/O
	mAudioNodes.setup();
}

void InputAnalyzer::resize()
{
	/*no op*/
}

void InputAnalyzer::update()
{
	mAudioNodes.update();
}

void InputAnalyzer::draw()
{
	// clear screen black
	ci::gl::clear();
	ci::gl::enableAlphaBlending();

	if (mGlobals.getThreadRenderer())
		mGlobals.getThreadRenderer()->draw();

	// draw interface GL params
	mParamsRef->draw();
	// draw FPS
	drawFps();

	ci::gl::disableAlphaBlending();
}

void InputAnalyzer::mouseDown(ci::app::MouseEvent event)
{
	mEventProcessor.processMouseEvents(
		static_cast<float>(event.getX()),
		static_cast<float>(event.getY()));
}

void InputAnalyzer::mouseDrag(ci::app::MouseEvent event)
{
	mEventProcessor.processMouseDragEvents(
		static_cast<float>(event.getX()),
		static_cast<float>(event.getY()),
		event.isLeftDown(),
		event.isMiddleDown(),
		event.isRightDown());
}

void InputAnalyzer::keyDown(ci::app::KeyEvent event)
{
	mEventProcessor.processKeybaordEvents(event.getChar());
}

void InputAnalyzer::drawFps()
{
	std::stringstream buf;
	buf << "FPS: " << ci::app::App::getAverageFps();
	ci::gl::drawStringRight(buf.str(), ci::Vec2i(ci::app::getWindowWidth() - 25, 10));
}

void InputAnalyzer::setupParamsGl()
{
	// make sure this is called only once
	// do note local statics gets instantiated once and only once.
	static std::once_flag isParamsSetup;
	// call it actually once
	std::call_once(isParamsSetup, [this] {
		mParamsRef = ci::params::InterfaceGl::create("Parameters", ci::Vec2i(50, 120));
		mParamsRef->hide(); //let's hide by default
		mEventProcessor.addKeyboardEvent([this](char c){ if (c == 'p' || c == 'P') {
			if (mParamsRef->isVisible())
				mParamsRef->hide();
			else
				mParamsRef->show();
		}});
		mGlobals.setParamsPtr(mParamsRef.get());
	});
}

} //!namespace cieq

CINDER_APP_NATIVE(cieq::InputAnalyzer, ci::app::RendererGl)