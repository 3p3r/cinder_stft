#include "app.h"
#include "color_pallete.h"

namespace cieq
{

namespace {
namespace GUI_STATICS {
const static std::string START_BUTTON("START");
const static std::string CONFIGURE_TEXT("Configure the parameters below and hit START");
const static std::string RECORD_TEXT("Record duration (s)");
const static std::string VIEWABLE_TEXT("Viewable Time range (s)");
const static std::string WINDOW_TEXT("Window duration (s)");
const static std::string HOP_TEXT("Hop duration (s)");
}}

InputAnalyzer::InputAnalyzer()
	: mGlobals(mEventProcessor, mWorkManager, mAudioNodes, mStftRenderer)
	, mAudioNodes(mGlobals)
	, mStftRenderer(mGlobals)
	, mMonitorRenderer(mGlobals)
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
	mAudioNodes.setupMonitor();
}

void InputAnalyzer::update()
{
	mStftRenderer.update();
	mAudioNodes.update();

	if (mAppConfig.shouldRemoveLaunchParams())
	{
		mGuiInstance->removeParam(GUI_STATICS::START_BUTTON);
		mGuiInstance->removeParam(GUI_STATICS::CONFIGURE_TEXT);
		mGuiInstance->setOptions(GUI_STATICS::RECORD_TEXT, "readonly=true");
		mGuiInstance->setOptions(GUI_STATICS::WINDOW_TEXT, "readonly=true");
		mGuiInstance->setOptions(GUI_STATICS::HOP_TEXT, "readonly=true");
		mGuiInstance->setOptions(GUI_STATICS::VIEWABLE_TEXT, "readonly=true");
		mAppConfig.LaunchParamsRemoved();

		mAudioNodes.setFormat(mAppConfig.getAsNodeFromat());
		mAudioNodes.setupRecorder();
		mStftRenderer.setup();
	}
}

void InputAnalyzer::draw()
{
	// clear screen black
	ci::gl::clear();
	ci::gl::disableDepthRead();
	ci::gl::disableDepthWrite();
	ci::gl::enableAlphaBlending();

	if (!mGlobals.getAudioNodes().isRecorderReady())
		mMonitorRenderer.draw();
	else
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

		mGuiInstance->addText(GUI_STATICS::CONFIGURE_TEXT);
		mGuiInstance->addParam(GUI_STATICS::RECORD_TEXT, &mAppConfig.mRecordDuration).min(300.0f).max(6000.0f).step(10.0f);
		mGuiInstance->addParam(GUI_STATICS::VIEWABLE_TEXT, &mAppConfig.mTimeRange).min(2.0f).max(20.0f).step(0.5f);
		mGuiInstance->addParam(GUI_STATICS::WINDOW_TEXT, &mAppConfig.mWindowDuration).min(0.01f).max(0.5f).step(0.01f);
		mGuiInstance->addParam(GUI_STATICS::HOP_TEXT, &mAppConfig.mHopDuration).min(0.005f).max(0.5f).step(0.005f);
		mGuiInstance->addParam("Low frequency (Hz)", &mAppConfig.mFrequencyLow).min(0.0f).max(25000.0f).step(100.0f);
		mGuiInstance->addParam("High frequency (Hz)", &mAppConfig.mFrequencyHigh).min(0.0f).max(25000.0f).step(100.0f);
		mGuiInstance->addSeparator();
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

		mGuiInstance->addButton(GUI_STATICS::START_BUTTON, [this] {
			mGuiInstance->minimize();
			mAppConfig.setRemoveLaunchParams();
		});
	});
}

} //!namespace cieq

CINDER_APP_NATIVE(cieq::InputAnalyzer, ci::app::RendererGl)