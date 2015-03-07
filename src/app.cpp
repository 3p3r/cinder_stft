#include "app.h"

namespace cieq
{

InputAnalyzer::InputAnalyzer()
	: mGlobals(mEventProcessor, mWorkManager, mAudioNodes, mStftRenderer)
	, mAudioNodes(mGlobals)
	, mStftRenderer(mGlobals)
{}

void InputAnalyzer::prepareSettings(Settings *settings)
{
	// We do not need console for this application
	settings->enableConsoleWindow(false);

	// It's ok if a user resizes the window, we'll respond accordingly
	settings->setResizable(true);

	// Title of the graphical window when it pops up
	settings->setTitle("Cinder STFT");

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
	// sets up ciUI
	setupGUI();
	// setup audio I/O
	mAudioNodes.setup();
	// setup STFT renderer
	mStftRenderer.setup();
}

void InputAnalyzer::resize()
{
	/*no op*/
}

void InputAnalyzer::update()
{
	mStftRenderer.update();
	mAudioNodes.update();

	if (mAppConfig.shouldRemoveLaunchParams())
	{
		mGuiInstance->removeParam("START");
		mAppConfig.LaunchParamsRemoved();
	}
}

void InputAnalyzer::draw()
{
	// clear screen black
	ci::gl::clear();
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	ci::gl::enableAlphaBlending();

	// draw STFT
	mStftRenderer.draw();

	// draw FPS
	drawFps();

	// draw GUI
	mGuiInstance->draw();

	ci::gl::disableAlphaBlending();
}

void InputAnalyzer::mouseDown(ci::app::MouseEvent event)
{
	mEventProcessor.fireMouseCallbacks(
		static_cast<float>(event.getX()),
		static_cast<float>(event.getY()));
}

void InputAnalyzer::keyDown(ci::app::KeyEvent event)
{
	mEventProcessor.fireKeyboardCallbacks(event.getChar());
}

void InputAnalyzer::drawFps()
{
	std::stringstream buf;
	buf << "FPS: " << ci::app::App::getAverageFps();
	ci::gl::drawStringRight(buf.str(), ci::Vec2i(ci::app::getWindowWidth() - 25, 10));
}

void InputAnalyzer::setupGUI()
{
	static std::once_flag __setup_gui_flag;
	std::call_once(__setup_gui_flag, [this]
	{
		mGuiInstance = ci::params::InterfaceGl::create("Cinder STFT parameters", ci::Vec2i(200, 350));
		mGuiInstance->addText("Portland State University, Winter 2015");
		mGuiInstance->addSeparator();

		mGuiInstance->addText("Developed by: Sepehr Laal, Raghad Boulos");
		mGuiInstance->addText("Instructor: Dr. James McNames", "fontsize=2");
		mGuiInstance->addSeparator();

		mGuiInstance->addText("This project performs STFT analysis on real time audio signal.");
		mGuiInstance->addSeparator();

		mGuiInstance->addText("Configure the parameters below and hit START");
		mGuiInstance->addParam("Record duration (s)", &mAppConfig.mRecordDuration, "min=300 max=6000");
		mGuiInstance->addParam("Viewable Time range (s)", &mAppConfig.mTimeRange, "min=2 max=20");
		mGuiInstance->addParam("Window duration (s)", &mAppConfig.mWindowDuration, "min=0.01 max=0.5");
		mGuiInstance->addParam("Hop duration (s)", &mAppConfig.mHopDuration, "min=0.01 max=0.5");
		mGuiInstance->addParam("Low frequency (Hz)", &mAppConfig.mFrequencyLow, "min=0 max=25000");
		mGuiInstance->addParam("High frequency (Hz)", &mAppConfig.mFrequencyHigh, "min=0 max=25000");
		mGuiInstance->addSeparator();

		mGuiInstance->addButton("START", [this] {
			mGuiInstance->minimize();
			mAppConfig.setRemoveLaunchParams();
		});
	});
}

} //!namespace cieq

CINDER_APP_NATIVE(cieq::InputAnalyzer, ci::app::RendererGl)