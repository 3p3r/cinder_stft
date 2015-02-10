#ifndef CIEQ_INCLUDE_AUDIO_DRAW_H_
#define CIEQ_INCLUDE_AUDIO_DRAW_H_

#include <cinder/Color.h>
#include <cinder/Vector.h>
#include <cinder/Rect.h>
#include <cinder/Surface.h>
#include <cinder/Font.h>

#include <vector>
#include <array>

namespace cinder {
namespace gl {
class TextureFont;
}} //!ci::gl

namespace cieq
{

class AudioNodes;

class Plot
{
public:
	Plot();
	virtual ~Plot() {};
	virtual void	setup();

	Plot&			setHorzAxisTitle(const std::string& title)	{ mHorzTitle = title; onHorzAxisTextChange(); return *this; }
	Plot&			setVertAxisTitle(const std::string& title)	{ mVertTitle = title; onVertAxisTextChange(); return *this; }
	Plot&			setVertAxisUnit(const std::string& unit)	{ mVertUnit = unit; onVertAxisTextChange(); return *this; }
	Plot&			setHorzAxisUnit(const std::string& unit)	{ mHorzUnit = unit; onHorzAxisTextChange(); return *this; }
	Plot&			setPlotTitle(const std::string& title)		{ mPlotTitle = title; return *this; }
	Plot&			setBounds(const ci::Rectf& bounds)			{ mBounds = bounds; return *this; }
	Plot&			setBoundsColor(const ci::ColorA& color)		{ mBoundsColor = color; return *this; }
	Plot&			setDrawBounds(bool on = true)				{ mDrawBounds = on; return *this; }
	Plot&			setDrawLabels(bool on = true)				{ mDrawLabels = on; return *this; }

	virtual	void	drawLocal() = 0;
	void			draw();
	void			drawBounds();
	void			drawLabels();

protected:
	std::string	mHorzTitle, mVertTitle;
	std::string	mHorzUnit, mVertUnit;
	std::string	mHorzText, mVertText;
	std::string	mPlotTitle;
	ci::Rectf	mBounds;
	ci::ColorA	mBoundsColor;
	bool		mDrawBounds, mDrawLabels;
	ci::Font	mLabelFont;

private:
	void			onHorzAxisTextChange();
	void			onVertAxisTextChange();
};

class WaveformPlot final : public Plot
{
public:
	WaveformPlot(AudioNodes& nodes);

	void					setGraphColor(const ci::ColorA& color);
	void					drawLocal() override;

private:
	ci::ColorA				mGraphColor;
	AudioNodes&				mAudioNodes;
};

class SpectrumPlot final : public Plot
{
public:
	SpectrumPlot(AudioNodes& nodes);

	void enableDecibelsScale(bool on = true);
	void drawLocal() override;

private:
	bool					mScaleDecibels;
	std::vector<ci::Vec2f>	mVerts;
	std::vector<ci::ColorA>	mColors;
	AudioNodes&				mAudioNodes;
};

class SpectrogramPlot final : public Plot
{
public:
	SpectrogramPlot(AudioNodes& nodes);

	void drawLocal() override;
	void setup() override;

private:
	AudioNodes&						mAudioNodes;
	std::array<ci::Surface32f, 2>	mSpectrals;
	std::size_t						mTexW, mTexH;
	int								mFrameCounter;
	int								mActiveSurface;
	int								mBackBufferSurface;
};

} //!namespace cieq

#endif //!CIEQ_INCLUDE_AUDIO_DRAW_H_