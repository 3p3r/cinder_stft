#include "audio_draw.h"
#include "audio_nodes.h"

#include <cinder/audio/Utilities.h>
#include <cinder/audio/MonitorNode.h>
#include <cinder/audio/SampleRecorderNode.h>
#include <cinder/Font.h>
#include <cinder/gl/gl.h>
#include <cinder/gl/TextureFont.h>
#include <cinder/app/App.h>
#include <cinder/Area.h>

#include "Resources.h"

namespace cieq
{

Plot::Plot()
	: mDrawBounds(true)
	, mBoundsColor(0.5f, 0.5f, 0.5f, 1)
{}

void Plot::draw()
{
	drawLocal();

	if (mDrawBounds)
	{
		drawBounds();
	}

	if (mDrawLabels)
	{
		drawLabels();
	}
}

void Plot::drawBounds()
{
	ci::gl::color(mBoundsColor);
	ci::gl::drawStrokedRect(mBounds);
}

void Plot::drawLabels()
{
	ci::gl::color(0, 0.9f, 0.9f);

	// draw x-axis label
	ci::gl::drawStringCentered(mHorzText, ci::Vec2f(mBounds.x1 + mBounds.getWidth() / 2.0f, mBounds.y2 + mLabelFont.getSize()), ci::ColorA::white(), mLabelFont);
	
	// draw plot title
	ci::gl::drawStringCentered(mPlotTitle, ci::Vec2f(mBounds.x1 + mBounds.getWidth() / 2.0f, mBounds.y1 - mLabelFont.getSize() * 2.0f), ci::ColorA::white(), mLabelFont);

	// draw y-axis label
	ci::gl::pushModelView();
	ci::gl::translate(mBounds.x1 - mLabelFont.getSize() * 2.0f, mBounds.y1 + mBounds.getHeight() / 2.0f);
	ci::gl::rotate(-90.0f);
	ci::gl::drawStringCentered(mVertText, ci::Vec2f::zero(), ci::ColorA::white(), mLabelFont);
	ci::gl::popModelView();
}

void Plot::onHorzAxisTextChange()
{
	mHorzText = mHorzTitle + " (" + mHorzUnit +")";
}

void Plot::onVertAxisTextChange()
{
	mVertText = mVertTitle + " (" + mVertUnit + ")";
}

void Plot::setup()
{
	mLabelFont = ci::Font(ci::app::loadResource(LABEL_FONT), 16);
}

SpectrumPlot::SpectrumPlot(AudioNodes& nodes)
	: mScaleDecibels( true )
	, mAudioNodes(nodes)
{
	setPlotTitle("FFT Analysis of input data");
	setHorzAxisTitle("Frequency").setHorzAxisUnit("Hz");
	setVertAxisTitle("Magnitude").setVertAxisUnit("Db");
}

// original draw function is from Cinder examples _audio/common
void SpectrumPlot::drawLocal()
{
	auto spectrum = mAudioNodes.getMonitorSpectralNode()->getMagSpectrum();
	
	if (spectrum.empty())
		return;

	ci::ColorA bottomColor(0, 0, 0.7f, 1);

	float width = mBounds.getWidth();
	float height = mBounds.getHeight();
	std::size_t numBins = spectrum.size();
	float padding = 0;
	float binWidth = ( width - padding * ( numBins - 1 ) ) / (float)numBins;

	std::size_t numVerts = spectrum.size() * 2 + 2;
	if( mVerts.size() < numVerts ) {
		mVerts.resize( numVerts );
		mColors.resize( numVerts );
	}

	std::size_t currVertex = 0;
	ci::Rectf bin(mBounds.x1, mBounds.y1, mBounds.x1 + binWidth, mBounds.y2);
	for( std::size_t i = 0; i < numBins; i++ ) {
		float m = spectrum[i];
		if( mScaleDecibels )
			m = ci::audio::linearToDecibel(m) / 100;

		bin.y1 = bin.y2 - m * height;

		mVerts[currVertex] = bin.getLowerLeft();
		mColors[currVertex] = bottomColor;
		mVerts[currVertex + 1] = bin.getUpperLeft();
		mColors[currVertex + 1] = ci::ColorA( 0, m, 0.7f, 1 );

		bin += ci::Vec2f(binWidth + padding, 0);
		currVertex += 2;
	}

	mVerts[currVertex] = bin.getLowerLeft();
	mColors[currVertex] = bottomColor;
	mVerts[currVertex + 1] = bin.getUpperLeft();
	mColors[currVertex + 1] = mColors[currVertex - 1];

	ci::gl::color(0, 0.9f, 0);

	glEnableClientState( GL_VERTEX_ARRAY );
	glEnableClientState( GL_COLOR_ARRAY );
	glVertexPointer( 2, GL_FLOAT, 0, mVerts.data() );
	glColorPointer( 4, GL_FLOAT, 0, mColors.data() );  // note: on OpenGL ES v1.1, the 'size' param to glColorPointer can only be 4
	glDrawArrays( GL_TRIANGLE_STRIP, 0, (GLsizei)mVerts.size() );
	glDisableClientState( GL_VERTEX_ARRAY );
	glDisableClientState( GL_COLOR_ARRAY );
}

void SpectrumPlot::enableDecibelsScale(bool on /*= true*/)
{
	mScaleDecibels = on;
}


WaveformPlot::WaveformPlot(AudioNodes& nodes)
	: mGraphColor(0, 0.9f, 0, 1)
	, mAudioNodes(nodes)
{
	setPlotTitle("RAW input data");
	setHorzAxisTitle("Time").setHorzAxisUnit("s");
	setVertAxisTitle("Amplitude").setVertAxisUnit("...");
}

void WaveformPlot::setGraphColor(const ci::ColorA& color)
{
	mGraphColor = color;
}

// original draw function is from Cinder examples _audio/common
void WaveformPlot::drawLocal()
{
	auto buffer = mAudioNodes.getMonitorNode()->getBuffer();

	ci::gl::color(mGraphColor);

	const float waveHeight = mBounds.getHeight() / (float)buffer.getNumChannels();
	const float xScale = mBounds.getWidth() / (float)buffer.getNumFrames();

	float yOffset = mBounds.y1;
	for (std::size_t ch = 0; ch < buffer.getNumChannels(); ch++) {
		ci::PolyLine2f waveform;
		const float *channel = buffer.getChannel(ch);
		float x = mBounds.x1;
		for (std::size_t i = 0; i < buffer.getNumFrames(); i++) {
			x += xScale;
			float y = (1 - (channel[i] * 0.5f + 0.5f)) * waveHeight + yOffset;
			waveform.push_back(ci::Vec2f(x, y));
		}

		if (!waveform.getPoints().empty())
			ci::gl::draw(waveform);

		yOffset += waveHeight;
	}
}


SpectrogramPlot::SpectrogramPlot(AudioNodes& nodes)
	: mAudioNodes(nodes)
	, mTexH(0)
	, mTexW(0)
	, mFrameCounter(0)
	, mActiveSurface(0)
	, mBackBufferSurface(1)
{
	setPlotTitle("Spectrogram");
	setHorzAxisTitle("Frequency").setHorzAxisUnit("Hz");
	setVertAxisTitle("Time").setVertAxisUnit("s");
}

void SpectrogramPlot::drawLocal()
{
	auto spectrum = mAudioNodes.getMonitorSpectralNode()->getMagSpectrum();

	if (spectrum.empty())
		return;

	auto surface_iter = mSpectrals[mActiveSurface].getIter();
	while (surface_iter.line())
	{
		if (surface_iter.mY != mFrameCounter) continue;
		while (surface_iter.pixel())
		{
			auto m = ci::audio::linearToDecibel(spectrum[surface_iter.mX]) / 100;
			surface_iter.r() = m;
			surface_iter.g() = m;
			surface_iter.b() = 1.0f - m;
		}
	}

	//mFrameCounter++;

	const auto height_offset = (mFrameCounter)* (mBounds.getHeight() / mTexH);
	const auto available_height = mBounds.getHeight() - height_offset;

	ci::gl::draw(mSpectrals[mActiveSurface], mBounds);
	ci::Area requested_area(0, mFrameCounter, mTexW, mTexH);
	ci::Rectf requested_rect(mBounds.x1, mBounds.y1 + height_offset, mBounds.x2, mBounds.y2);
	ci::gl::draw(mTexCache, requested_area, requested_rect);

	mFrameCounter++;


	if (mFrameCounter >= mTexH)
	{
		mFrameCounter = 0;
		std::swap(mActiveSurface, mBackBufferSurface);
		mTexCache.update(mSpectrals[mBackBufferSurface]);
	}
}

void SpectrogramPlot::setup()
{
	Plot::setup();
	mTexW = mAudioNodes.getMonitorSpectralNode()->getNumBins();
	//swap out the texture every 3 seconds
	mTexH = static_cast<std::size_t>(ci::app::getFrameRate() * 3.0f);

	mSpectrals[0] = ci::Surface32f(mTexW, mTexH, false);
	mSpectrals[1] = ci::Surface32f(mTexW, mTexH, false);

	mTexCache = ci::gl::Texture(mSpectrals.back());
}


ContiguousWaveformPlot::ContiguousWaveformPlot(AudioNodes& nodes)
	: mGraphColor(0, 0.9f, 0, 1)
	, mAudioNodes(nodes)
	, mSampleToSkip(0)
{
	setPlotTitle("Recorded input data");
	setHorzAxisTitle("Time").setHorzAxisUnit("s");
	setVertAxisTitle("Sample").setVertAxisUnit("units");
}

void ContiguousWaveformPlot::setGraphColor(const ci::ColorA& color)
{
	mGraphColor = color;
}

void ContiguousWaveformPlot::drawLocal()
{
	auto points = mAudioNodes.getBufferRecorderNode()->getRecordedCopy();

	ci::gl::color(mGraphColor);

	const float waveHeight = mBounds.getHeight() / (float)points->getNumChannels();
	const float xScale = 1.0f;

	float yOffset = mBounds.y1;
	for (std::size_t ch = 0; ch < points->getNumChannels(); ch++)
	{
		const float *channel = points->getChannel(ch);
		for (std::size_t i = 0; i < mAudioNodes.getBufferRecorderNode()->getWritePosition(); i += mSampleToSkip)
		{
			if (mGraphs[ch].size() <= i / mSampleToSkip)
			{
				float x = mBounds.x1 + mGraphs[ch].getPoints().size();
				float y = (1 - (channel[i] * 0.5f + 0.5f));
				
				for (std::size_t j = i; i > mSampleToSkip && j > i - mSampleToSkip; --j)
				{
					y = (y + (1 - (channel[j] * 0.5f + 0.5f))) / 2.0f;
				}

				y *= waveHeight;
				if (ci::math<float>::abs(y) > waveHeight) y = 0; //fix me
				y += yOffset;
				
				mGraphs[ch].push_back(ci::Vec2f(x, y));
			}
		}

		if (!mGraphs[ch].getPoints().empty())
			ci::gl::draw(mGraphs[ch]);

		yOffset += waveHeight;
	}
}

void ContiguousWaveformPlot::setup()
{
	Plot::setup();
	auto recorder = mAudioNodes.getBufferRecorderNode();
	mGraphs.resize(mAudioNodes.getBufferRecorderNode()->getNumChannels());
	mSampleToSkip = static_cast<std::size_t>(recorder->getNumFrames() / mBounds.getWidth());
}

} //!namespace cieq