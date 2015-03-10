#include "app.h"
#include "palette_manager.h"

namespace cieq
{

InputAnalyzer::InputAnalyzer()
	: mGlobals(mEventProcessor, mWorkManager, mAudioNodes, mStftRenderer, mGridRenderer, mAppConfig, mFilter)
	, mFilter(mGlobals)
	, mAudioNodes(mGlobals)
	, mStftRenderer(mGlobals)
	, mMonitorRenderer(mGlobals)
	, mGridRenderer(mGlobals)
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

	// cap at 300.0 fps! yeaaaah juju :)
	settings->setFrameRate(300.0f);
}

void InputAnalyzer::setup()
{
	// sets up GUI
	setupGUI();
	// setup audio I/O
	mAudioNodes.setupInput();
	mAudioNodes.setupMonitor();
}

void InputAnalyzer::update()
{
	mStftRenderer.update();
	mAudioNodes.update();

	if (mAppConfig.shouldRemoveLaunchParams())
	{
		mAppConfig.removeFromGui(mGuiInstance.get());

		mGuiInstance->addSeparator();
		mGuiInstance->addText("Grid options:");
		mGuiInstance->addParam("Draw grids?", &mGridRenderer.mConfiguration.mVisible);
		mGuiInstance->addParam("Grid Color", &mGridRenderer.mConfiguration.mGridColor);
		mGuiInstance->addParam("Step X", &mGridRenderer.mConfiguration.mStepX).min(10).max(100);
		mGuiInstance->addParam("Step Y", &mGridRenderer.mConfiguration.mStepY).min(10).max(100);
		mGuiInstance->addParam("Label Frequency", &mGridRenderer.mConfiguration.mLabelFrequency).min(1).max(10);
		mGuiInstance->addParam("Label Margin", &mGridRenderer.mConfiguration.mLabelMargin);
		mGuiInstance->addParam("Label Color", &mGridRenderer.mConfiguration.mLabelColor);

		mFilter.removeFromGui(mGuiInstance.get());

		mAppConfig.LaunchParamsRemoved();

		mAudioNodes.setFormat(mAppConfig.getAsNodeFromat());
		mAudioNodes.setupRecorder();
		mStftRenderer.setup();
	}
}

void InputAnalyzer::draw()
{
	// clear screen black
	if (mGlobals.getAudioNodes().isRecorderReady())
		ci::gl::clear(palette::Manager::instance().getActivePaletteMinColor());
	else
		ci::gl::clear();
	
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	ci::gl::enableAlphaBlending();

	if (!mGlobals.getAudioNodes().isRecorderReady())
		mMonitorRenderer.draw();
	else
		mStftRenderer.draw();

	// draw grid
	mGridRenderer.draw();

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
		mGuiInstance = ci::params::InterfaceGl::create("Cinder STFT parameters", ci::Vec2i(425, 500));
		mGuiInstance->addText("Portland State University, Winter 2015");
		mGuiInstance->addSeparator();
		// -----------------------------------------------

		mGuiInstance->addText("Developed by: Sepehr Laal, Raghad Boulos");
		mGuiInstance->addText("Instructor: Dr. James McNames");
		mGuiInstance->addSeparator();
		// -----------------------------------------------

		mGuiInstance->addText("This project performs STFT analysis on real time audio signal.");
		mGuiInstance->addSeparator();
		// -----------------------------------------------

		mAppConfig.addToGui(mGuiInstance.get());
		// -----------------------------------------------

		mFilter.addToGui(mGuiInstance.get());
		// -----------------------------------------------

		mGuiInstance->addText("Palette settings. Type:");
		mGuiInstance->addText("0 --> Matlab JET");
		mGuiInstance->addText("1 --> Matlab HOT");
		mGuiInstance->addText("2 --> MPL Summer");
		mGuiInstance->addText("3 --> MPL Paired");
		mGuiInstance->addText("4 --> MPL Ocean");
		mGuiInstance->addText("5 --> MPL Winter");

		mGuiInstance->addParam<int>("Color palette",
			[](int p){ palette::Manager::instance().setActivePalette(p); },
			[]()->int{ return palette::Manager::instance().getActivePalette(); });

		mGuiInstance->addParam<bool>("Convert to dB mode",
			[](bool c){ palette::Manager::instance().setConvertToDb(c); },
			[]()->bool{ return palette::Manager::instance().getConvertToDb(); });
		
		mGuiInstance->addParam<float>("dB mode divisor",
			[](float val){ palette::Manager::instance().setDbDivisor(val); },
			[]()->float{ return palette::Manager::instance().getDbDivisor(); });
		
		mGuiInstance->addParam<float>("linear mode coefficient",
			[](float val){ palette::Manager::instance().setLinearCoefficient(val); },
			[]()->float{ return palette::Manager::instance().getLinearCoefficient(); });
		
		mGuiInstance->addParam<float>("Min color threshold [0, 1]",
			[](float val){ palette::Manager::instance().setMinThreshold(val); },
			[]()->float{ return palette::Manager::instance().getMinThreshold(); });
		
		mGuiInstance->addParam<float>("Max color threshold [0, 1]",
			[](float val){ palette::Manager::instance().setMaxThreshold(val); },
			[]()->float{ return palette::Manager::instance().getMaxThreshold(); });
		mGuiInstance->addSeparator();
		// -----------------------------------------------
	});
}

} //!namespace cieq

CINDER_APP_NATIVE(cieq::InputAnalyzer, ci::app::RendererGl)