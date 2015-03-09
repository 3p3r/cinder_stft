#include "grid_renderer.h"
#include "app_globals.h"
#include "audio_nodes.h"

#include <cinder/Color.h>
#include <cinder/app/App.h>

namespace cieq {

GridRenderer::GridRenderer(AppGlobals& g)
	: mGlobals(g)
{
	resetConfiguration();
}

void GridRenderer::draw()
{
	if (!mConfiguration.mVisible || !mGlobals.getAudioNodes().isRecorderReady()) return;

	ci::gl::SaveColorState _save_color;
	ci::gl::color(mConfiguration.mGridColor);

	const auto _w_float = static_cast<float>(ci::app::getWindowWidth());
	const auto _h_float = static_cast<float>(ci::app::getWindowHeight());

	int count = 0;
	// draw horizontal axis
	for (int i = mConfiguration.mStepY; i <= ci::app::getWindowHeight(); i += mConfiguration.mStepY)
	{
		const auto _y = static_cast<float>(i);
		ci::gl::drawLine(ci::Vec2f(0.0f, _y), ci::Vec2f(_w_float, _y));

		if (count % mConfiguration.mLabelFrequency == 0) // every 4 steps, draw string of where we're standing
		{
			const auto _val = ((i / _h_float) * (mConfiguration.mMaxY - mConfiguration.mMinY)) + mConfiguration.mMinY;
			ci::gl::drawStringCentered(std::to_string(_val), ci::Vec2f(mConfiguration.mLabelMargin, _y), mConfiguration.mLabelColor);
		}

		count++;
	}

	count = 0;
	// draw vertical axis
	for (int i = mConfiguration.mStepX; i <= ci::app::getWindowWidth(); i += mConfiguration.mStepX)
	{
		const auto _x = static_cast<float>(i);
		ci::gl::drawLine(ci::Vec2f(_x, 0.0f), ci::Vec2f(_x, _h_float));

		if (count % mConfiguration.mLabelFrequency == 0) // every 4 steps, draw string of where we're standing
		{
			ci::gl::pushMatrices();
			ci::gl::rotate(-90.0f);
			const auto _val = ((i / _w_float) * (mConfiguration.mMaxX - mConfiguration.mMinX)) + mConfiguration.mMinX;
			ci::gl::drawStringCentered(std::to_string(_val), ci::Vec2f((mConfiguration.mLabelMargin - ci::app::getWindowHeight()), _x), mConfiguration.mLabelColor);
			ci::gl::popMatrices();
		}

		count++;
	}

}

void GridRenderer::resetConfiguration()
{
	mConfiguration.mLabelFrequency = 4;
	mConfiguration.mGridColor = ci::Color(0.2f, 0.2f, 0.2f);
	mConfiguration.mLabelColor = ci::Color::white();
	mConfiguration.mLabelMargin = 25.0f;
	mConfiguration.mMaxX = 1.0f;
	mConfiguration.mMaxY = 1.0f;
	mConfiguration.mMinX = 0.0f;
	mConfiguration.mMinY = 0.0f;
	mConfiguration.mStepX = 50;
	mConfiguration.mStepY = 50;
	mConfiguration.mVisible = false;
}

}