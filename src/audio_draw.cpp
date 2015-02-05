/*
 Copyright (c) 2014, The Cinder Project

 This code is intended to be used with the Cinder C++ library, http://libcinder.org

 Redistribution and use in source and binary forms, with or without modification, are permitted provided that
 the following conditions are met:

    * Redistributions of source code must retain the above copyright notice, this list of conditions and
	the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice, this list of conditions and
	the following disclaimer in the documentation and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
 TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 POSSIBILITY OF SUCH DAMAGE.
*/

#include "audio_draw.h"

#include <cinder/audio/Utilities.h>

#include <cinder/CinderMath.h>
#include <cinder/Triangulate.h>
#include <cinder/gl/gl.h>

namespace cieq
{

void drawAudioBuffer( const ci::audio::Buffer &buffer, const ci::Rectf &bounds, bool drawFrame, const ci::ColorA &color )
{
	ci::gl::color( color );

	const float waveHeight = bounds.getHeight() / (float)buffer.getNumChannels();
	const float xScale = bounds.getWidth() / (float)buffer.getNumFrames();

	float yOffset = bounds.y1;
	for (std::size_t ch = 0; ch < buffer.getNumChannels(); ch++) {
		ci::PolyLine2f waveform;
		const float *channel = buffer.getChannel( ch );
		float x = bounds.x1;
		for( std::size_t i = 0; i < buffer.getNumFrames(); i++ ) {
			x += xScale;
			float y = ( 1 - ( channel[i] * 0.5f + 0.5f ) ) * waveHeight + yOffset;
			waveform.push_back( ci::Vec2f( x, y ) );
		}

		if( ! waveform.getPoints().empty() )
			ci::gl::draw( waveform );

		yOffset += waveHeight;
	}

	if( drawFrame ) {
		ci::gl::color( color.r, color.g, color.b, color.a * 0.6f );
		ci::gl::drawStrokedRect(bounds);
	}
}

// ----------------------------------------------------------------------------------------------------
// MARK: - WaveformPlot
// ----------------------------------------------------------------------------------------------------

namespace {

inline void calcMinMaxForSection( const float *buffer, std::size_t samplesPerSection, float &max, float &min ) {
	max = 0;
	min = 0;
	for( size_t k = 0; k < samplesPerSection; k++ ) {
		float s = buffer[k];
		max = ci::math<float>::max(max, s);
		min = ci::math<float>::min(min, s);
	}
}

inline void calcAverageForSection( const float *buffer, std::size_t samplesPerSection, float &upper, float &lower ) {
	upper = 0;
	lower = 0;
	for( std::size_t k = 0; k < samplesPerSection; k++ ) {
		float s = buffer[k];
		if( s > 0 ) {
			upper += s;
		} else {
			lower += s;
		}
	}
	upper /= samplesPerSection;
	lower /= samplesPerSection;
}

} // anonymous namespace

void Waveform::load( const float *samples, std::size_t numSamples, const ci::Vec2i &waveSize, std::size_t pixelsPerVertex, CalcMode mode )
{
    float height = waveSize.y / 2.0f;
    size_t numSections = waveSize.x / pixelsPerVertex + 1;
    size_t samplesPerSection = numSamples / numSections;

	std::vector<ci::Vec2f> &points = mOutline.getPoints();
	points.resize( numSections * 2 );

    for( size_t i = 0; i < numSections; i++ ) {
		float x = (float)i * pixelsPerVertex;
		float yUpper, yLower;
		if( mode == CalcMode::MIN_MAX ) {
			calcMinMaxForSection( &samples[i * samplesPerSection], samplesPerSection, yUpper, yLower );
		} else {
			calcAverageForSection( &samples[i * samplesPerSection], samplesPerSection, yUpper, yLower );
		}
		points[i] = ci::Vec2f(x, height - height * yUpper);
		points[numSections * 2 - i - 1] = ci::Vec2f(x, height - height * yLower);
    }
	mOutline.setClosed();

	mMesh = ci::Triangulator(mOutline).calcMesh();
}


void WaveformPlot::load( const std::vector<float> &samples, const ci::Rectf &bounds, std::size_t pixelsPerVertex )
{
	mBounds = bounds;
	mWaveforms.clear();

	ci::Vec2i waveSize = bounds.getSize();
	mWaveforms.push_back( Waveform( samples, waveSize, pixelsPerVertex, Waveform::CalcMode::MIN_MAX ) );
	mWaveforms.push_back( Waveform( samples, waveSize, pixelsPerVertex, Waveform::CalcMode::AVERAGE ) );
}

void WaveformPlot::load(const ci::audio::BufferRef &buffer, const ci::Rectf &bounds, std::size_t pixelsPerVertex)
{
	mBounds = bounds;
	mWaveforms.clear();

	std::size_t numChannels = buffer->getNumChannels();
	ci::Vec2i waveSize = bounds.getSize();
	waveSize.y /= numChannels;
	for( std::size_t ch = 0; ch < numChannels; ch++ ) {
		mWaveforms.push_back( Waveform( buffer->getChannel( ch ), buffer->getNumFrames(), waveSize, pixelsPerVertex, Waveform::CalcMode::MIN_MAX ) );
		mWaveforms.push_back( Waveform( buffer->getChannel( ch ), buffer->getNumFrames(), waveSize, pixelsPerVertex, Waveform::CalcMode::AVERAGE ) );
	}
}

void WaveformPlot::draw()
{
	auto &waveforms = getWaveforms();
	if( waveforms.empty() ) {
		return;
	}

	ci::gl::color(mColorMinMax);
	ci::gl::draw(waveforms[0].getMesh());

	ci::gl::color(mColorAverage);
	ci::gl::draw(waveforms[1].getMesh());

	if( waveforms.size() > 2 ) {
		ci::gl::pushMatrices();
		ci::gl::translate(0, getBounds().getHeight() / 2);

		ci::gl::color(mColorMinMax);
		ci::gl::draw(waveforms[2].getMesh());

		ci::gl::color(mColorAverage);
		ci::gl::draw(waveforms[3].getMesh());
		
		ci::gl::popMatrices();
	}
}

// ----------------------------------------------------------------------------------------------------
// MARK: - SpectrumPlot
// ----------------------------------------------------------------------------------------------------

SpectrumPlot::SpectrumPlot()
: mScaleDecibels( true ), mBorderEnabled( true ), mBorderColor( 0.5f, 0.5f, 0.5f, 1 )
{
}

void SpectrumPlot::draw( const std::vector<float> &magSpectrum )
{
	if( magSpectrum.empty() )
		return;

	ci::ColorA bottomColor(0, 0, 0.7f, 1);

	float width = mBounds.getWidth();
	float height = mBounds.getHeight();
	std::size_t numBins = magSpectrum.size();
	float padding = 0;
	float binWidth = ( width - padding * ( numBins - 1 ) ) / (float)numBins;

	std::size_t numVerts = magSpectrum.size() * 2 + 2;
	if( mVerts.size() < numVerts ) {
		mVerts.resize( numVerts );
		mColors.resize( numVerts );
	}

	std::size_t currVertex = 0;
	ci::Rectf bin(mBounds.x1, mBounds.y1, mBounds.x1 + binWidth, mBounds.y2);
	for( std::size_t i = 0; i < numBins; i++ ) {
		float m = magSpectrum[i];
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

	if( mBorderEnabled ) {
		ci::gl::color(mBorderColor);
		ci::gl::drawStrokedRect(mBounds);
	}
}

} //!namespace cieq