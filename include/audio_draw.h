#ifndef CIEQ_INCLUDE_AUDIO_DRAW_H_
#define CIEQ_INCLUDE_AUDIO_DRAW_H_

#include <cinder/Color.h>
#include <cinder/Vector.h>
#include <cinder/Rect.h>

#include <vector>

namespace cieq
{

class AudioNodes;

class Plot
{
public:
	Plot();
	virtual ~Plot() {};

	Plot&			setHorzAxisTitle(const std::string& title)	{ mHorzTitle = title; return *this; }
	Plot&			setVertAxisTitle(const std::string& title)	{ mVertTitle = title; return *this; }
	Plot&			setVertAxisUnit(const std::string& unit)	{ mVertUnit = unit; return *this; }
	Plot&			setHorzAxisUnit(const std::string& unit)	{ mHorzUnit = unit; return *this; }
	Plot&			setBounds(const ci::Rectf& bounds)			{ mBounds = bounds; return *this; }
	Plot&			setBoundsColor(const ci::ColorA& color)		{ mBoundsColor = color; return *this; }
	Plot&			drawBounds(bool on = true)					{ mDrawBounds = on; return *this; }

	virtual	void	drawLocal() = 0;
	void			draw();

protected:
	std::string	mHorzTitle, mVertTitle;
	std::string	mHorzUnit, mVertUnit;
	ci::Rectf	mBounds;
	ci::ColorA	mBoundsColor;
	bool		mDrawBounds;
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

} //!namespace cieq

#endif //!CIEQ_INCLUDE_AUDIO_DRAW_H_