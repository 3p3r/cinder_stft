#include "monitor_renderer.h"
#include "app_globals.h"
#include "audio_nodes.h"

#include <cinder/audio/MonitorNode.h>
#include <cinder/gl/gl.h>
#include <cinder/app/App.h>

namespace cistft {

MonitorRenderer::MonitorRenderer(AppGlobals& g)
	: mGlobals(g)
{}

void MonitorRenderer::draw()
{
	if (!mGlobals.getAudioNodes().isMonitorReady()) return;

	ci::gl::SaveColorState _save_color;

	auto buffer = mGlobals.getAudioNodes().getMonitorNode()->getBuffer();

	ci::gl::lineWidth(3.0f);
	ci::gl::color(0.65f, 0.80f, 0.44f, 1);

	const float waveHeight = ci::app::getWindowHeight() / (float)buffer.getNumChannels();
	const float xScale = ci::app::getWindowWidth() / (float)buffer.getNumFrames();

	float yOffset = 0.0f;
	for (std::size_t ch = 0; ch < buffer.getNumChannels(); ch++) {
		ci::PolyLine2f waveform;
		const float *channel = buffer.getChannel(ch);
		float x = 0.0f;
		for (std::size_t i = 0; i < buffer.getNumFrames(); i++) {
			x += xScale;
			float y = (1 - (channel[i] * 0.5f + 0.5f)) * waveHeight + yOffset;
			waveform.push_back(ci::Vec2f(x, y));
		}

		if (!waveform.getPoints().empty())
			ci::gl::draw(waveform);

		yOffset += waveHeight;
	}

	ci::gl::lineWidth(1.0f); //restore
}

}