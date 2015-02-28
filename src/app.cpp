#include "app.h"

#include <cinder/params/Params.h>

namespace cieq
{

InputAnalyzer::InputAnalyzer()
	: mGlobals(mEventProcessor, mWorkManager, mAudioNodes)
	, mAudioNodes(mGlobals)
	, mSpectrumPlot(mAudioNodes)
	, mWaveformPlot(mAudioNodes)
	, mSpectrogramPlot(mAudioNodes)
	, mContiguousPlot(mAudioNodes)
	, mCameras(mGlobals)
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
	// setup camera rig
	mCameras.setup();
	// setup the interface GL params
	setupParamsGl();
	// setup audio I/O
	mAudioNodes.setup();
	// position the plots
	positionPlots();
	if (mAudioNodes.ready())
	{
		// setup plots
		mSpectrumPlot.setup();
		mWaveformPlot.setup();
		mContiguousPlot.setup();
		mSpectrogramPlot.setup();
	}
}

void InputAnalyzer::resize()
{
	// re position the plots
	positionPlots();
	// tell camera we've resized
	mCameras.resize();
}

void InputAnalyzer::update()
{
	mAudioNodes.update();
}

void InputAnalyzer::draw()
{
	// clear screen black
	ci::gl::clear();
	// enable alpha channel
	ci::gl::enableAlphaBlending();
	if (mAudioNodes.ready())
	{
		// draw plots
		mSpectrumPlot.draw();

		if (mGlobals.getDrawContiguous())
			mContiguousPlot.draw();
		else
			mWaveformPlot.draw();

		mSpectrogramPlot.draw();
	}
	// draw interface GL params
	mParamsRef->draw();
	// draw FPS
	drawFps();
	// disable alpha channel and exit
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

void InputAnalyzer::positionPlots()
{
	const auto window_size = ci::app::getWindowSize();
	const auto plot_size_width = 0.9f * window_size.x; // 90% of window width
	const auto plot_size_width_mini = 0.425f * window_size.x; // 42.5% of window width
	const auto plot_padding_horz = 0.05f * window_size.x; // 5% of window width
	const auto plot_size_height = 0.8f * 0.5f * window_size.y; // half of 90% of window width

	ci::Vec2f top_left(0.05f * window_size.x, 0.05f * window_size.y);
	mSpectrumPlot.setBounds(ci::Rectf(top_left, top_left + ci::Vec2f(plot_size_width, plot_size_height)));
	top_left.y += 0.5f * window_size.y;
	mWaveformPlot.setBounds(ci::Rectf(top_left, top_left + ci::Vec2f(plot_size_width_mini, plot_size_height)));
	mContiguousPlot.setBounds(ci::Rectf(top_left, top_left + ci::Vec2f(plot_size_width_mini, plot_size_height)));
	top_left.x += plot_size_width_mini + plot_padding_horz;
	mSpectrogramPlot.setBounds(ci::Rectf(top_left, top_left + ci::Vec2f(plot_size_width_mini, plot_size_height)));
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