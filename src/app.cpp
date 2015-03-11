#include "app.h"
#include "palette_manager.h"

namespace cistft
{

Application::Application()
	: mGlobals(mWorkManager, mAudioNodes, mStftRenderer, mGridRenderer, mAppConfig)
	, mAudioNodes(mGlobals)
	, mStftRenderer(mGlobals)
	, mMonitorRenderer(mGlobals)
	, mGridRenderer(mGlobals)
{}

void Application::prepareSettings(Settings *settings)
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

	// cap at 300.0 fps!
	settings->setFrameRate(300.0f);
}

void Application::setup()
{
	// sets up pre-launch GUI
	setupPreLaunchGUI();
	// setup audio input from microphone
	mAudioNodes.setupInput();
	// setup basic waveform monitor input, used to plot
	// the real-time pre-launch audio waveform.
	mAudioNodes.setupMonitor();
}

void Application::update()
{
	// Update STFT renderer (a no-op if it's not setup)
	mStftRenderer.update();
	// Update audio input and pull the latest data in
	mAudioNodes.update();
	// Check if we have to swap GUI with post-launch one
	if (mAppConfig.shouldLaunch())
		setupPostLaunchGUI();
}

void Application::draw()
{
	// clear screen black
	ci::gl::clear();
	
	// we're not 3D rendering so turn it off
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();

	// enable alpha blending
	ci::gl::enableAlphaBlending();

	// if recorder is not recording then...
	if (!mGlobals.getAudioNodes().isRecorderReady())
	{
		// render pre-launch waveform
		mMonitorRenderer.draw();
	}
	else
	{
		// render STFT data
		mStftRenderer.draw();
	}

	// draw grid
	mGridRenderer.draw();

	// draw FPS
	drawFps();

	// draw GUI
	mGuiInstance->draw();

	// Turn off Alpha blending
	ci::gl::disableAlphaBlending();
}

void Application::mouseDown(ci::app::MouseEvent event)
{
	// If user clicks anywhere on screen
	if (mAudioNodes.isInputReady())
	{
		mAudioNodes.toggleInput();
	}
}

void Application::keyDown(ci::app::KeyEvent event)
{
	// If user enters SPACE
	if (event.getChar() == ' ')
	{
		if (mAudioNodes.isInputReady())
		{
			mAudioNodes.toggleInput();
		}
	}
}

void Application::drawFps()
{
	std::stringstream buf;
	buf << "FPS: " << ci::app::App::getAverageFps();
	ci::gl::drawStringRight(buf.str(), ci::Vec2i(ci::app::getWindowWidth() - 25, 10));
}

void Application::setupPreLaunchGUI()
{
	static std::once_flag __setup_gui_flag;
	std::call_once(__setup_gui_flag, [this]
	{
		mGuiInstance = ci::params::InterfaceGl::create("Cinder STFT parameters", ci::Vec2i(425, 500));
		// -----------------------------------------------
		mGuiInstance->addText("Portland State University, Winter 2015");
		mGuiInstance->addSeparator();
		// -----------------------------------------------
		mGuiInstance->addText("Developed by: Sepehr Laal and Raghad Boulos");
		mGuiInstance->addText("Instructor: Dr. James McNames");
		mGuiInstance->addSeparator();
		// -----------------------------------------------
		mGuiInstance->addText("This project performs STFT analysis on real time audio signal.");
		mGuiInstance->addSeparator();
		// -----------------------------------------------
		mAppConfig.setupPreLaunchGUI(mGuiInstance.get());
		mGridRenderer.setupPreLaunchGUI(mGuiInstance.get());
	});
}

void Application::setupPostLaunchGUI()
{
	static std::once_flag __setup_gui_flag;
	std::call_once(__setup_gui_flag, [this]
	{
		mAppConfig.setupPostLaunchGUI(mGuiInstance.get());
		mAppConfig.performLaunch();

		mGridRenderer.setupPostLaunchGUI(mGuiInstance.get());
		mGridRenderer.setHorizontalUnit("Hz");
		mGridRenderer.setVerticalUnit("s");
		mGridRenderer.setVerticalBoundary(mAppConfig.getActualHighPassFrequency(), mAppConfig.getActualLowPassFrequency());
		
		mAudioNodes.setupRecorder();
		mStftRenderer.setup();
	});
}

} //!namespace cistft

CINDER_APP_NATIVE(cistft::Application, ci::app::RendererGl)