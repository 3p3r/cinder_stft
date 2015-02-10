#ifndef CIEQ_INCLUDE_APP_H_
#define CIEQ_INCLUDE_APP_H_

#include <cinder/app/AppNative.h>

#include "app_globals.h"
#include "app_event.h"
#include "audio_nodes.h"
#include "audio_draw.h"

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

	// \note Tells Cinder how to prepare the window
	void		prepareSettings(Settings *settings) override final;

	void		setup() override final;
	void		update() override final;
	void		draw() override final;
	void		resize() override final;
	void		shutdown() override final;
	void		positionPlots();
	void		drawFps();

	void		mouseDown(ci::app::MouseEvent event) override final;
	void		keyDown(ci::app::KeyEvent event) override final;

	AppGlobals&	getGlobals() { return mGlobals; }

private:
	AppEvent		mEventProcessor;
	AppGlobals		mGlobals;
	AudioNodes		mAudioNodes;
	SpectrumPlot	mSpectrumPlot;
	WaveformPlot	mWaveformPlot;
	SpectrogramPlot	mSpectrogramPlot;
	std::shared_ptr<ci::params::InterfaceGl>
					mParamsRef;
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_H_