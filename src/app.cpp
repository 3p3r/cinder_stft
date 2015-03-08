#include "app.h"
#include "color_pallete.h"

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
	mAudioNodes.setupInput();
	// setup STFT renderer
	//mStftRenderer.setup();
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

		mGuiInstance->addButton("START", [this] {
			mGuiInstance->minimize();
			mAppConfig.setRemoveLaunchParams();
		});
	});
}

} //!namespace cieq

CINDER_APP_NATIVE(cieq::InputAnalyzer, ci::app::RendererGl)