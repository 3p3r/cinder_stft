#include "stft_filter.h"
#include "app_globals.h"
#include "app_config.h"
#include "audio_nodes.h"

#include <cinder/audio/Context.h>
#include <cinder/CinderMath.h>
#include <cinder/params/Params.h>

namespace cieq {

StftFilter::StftFilter(AppGlobals& g)
	: mViewableBins(256)
	, mLowPassFrequency(10000.0f) //10KHz
	, mHighPassFrequency(100.0f) //100Hz
	, mCalculatedFftSize(0)
	, mActualLowPassFrequency(0)
	, mActualHighPassFrequency(0)
	, mMagnitudeIndexEnd(0)
	, mMagnitudeIndexStart(0)
	, mGlobals(g)
{}

void StftFilter::calculate()
{
	// If not a valid bandpass
	if (mLowPassFrequency < mHighPassFrequency)
		mLowPassFrequency = mHighPassFrequency + 5000.0f;

	// just a sanity check
	if (mViewableBins < 0)
		mViewableBins = 256;

	mCalculatedFftSize = static_cast<int>(((mViewableBins - 1) * ci::audio::Context::master()->getSampleRate()) / (mLowPassFrequency - mHighPassFrequency));

	if (!ci::isPowerOf2(mCalculatedFftSize))
		mCalculatedFftSize = ci::nextPowerOf2(mCalculatedFftSize);

	if (mCalculatedFftSize <= static_cast<int>(mGlobals.getAppConfig().getAsNodeFromat().getWindowDurationInSamples()))
	{
		while (mCalculatedFftSize <= static_cast<int>(mGlobals.getAppConfig().getAsNodeFromat().getWindowDurationInSamples()))
		{
			mCalculatedFftSize = ci::nextPowerOf2(mCalculatedFftSize);
		}
	}

	// Less than 1024? get out of here.
	if (mCalculatedFftSize < 1024) mCalculatedFftSize = 1024;

	auto _temp_frequency = 0.0f;
	int index = 0;
	for (index = 0; index < mCalculatedFftSize / 2 && _temp_frequency < mHighPassFrequency; ++index)
	{
		_temp_frequency += index * (1.0f / mCalculatedFftSize);
	}
	
	if (_temp_frequency > mHighPassFrequency)
	{
		_temp_frequency -= index * (1.0f / mCalculatedFftSize);
		index--;
	}

	mMagnitudeIndexStart = index;
	mMagnitudeIndexEnd = index + (mViewableBins - 1);

	if (mMagnitudeIndexEnd >= mCalculatedFftSize / 2)
	{
		// this should never happen but just in case.
		mMagnitudeIndexEnd = (mCalculatedFftSize / 2) - 1;
	}

	mActualHighPassFrequency = _temp_frequency;
	mActualLowPassFrequency = (mViewableBins * (1.0f / mCalculatedFftSize)) + _temp_frequency;
}

StftFilter& StftFilter::viewableBins(int val)
{
	mViewableBins = val;
	calculate();
	return *this;
}

StftFilter& StftFilter::lowPassFrequency(float val)
{
	if (ci::audio::Context::master()->getSampleRate() / 2.0f < val)
	{
		val = ci::audio::Context::master()->getSampleRate() / 2.0f;
	}

	mLowPassFrequency = val;
	calculate();
	return *this;
}

StftFilter& StftFilter::highPassFrequency(float val)
{
	if (val < 0.0f) val = 0.0f;

	mHighPassFrequency = val;
	calculate();
	return *this;
}

int StftFilter::getViewableBins() const
{
	return mViewableBins;
}

float StftFilter::getLowPassFrequency() const
{
	return mLowPassFrequency;
}

float StftFilter::getHighPassFrequency() const
{
	return mHighPassFrequency;
}

int StftFilter::getCalculatedFftSize() const
{
	return mCalculatedFftSize;
}

float StftFilter::getActualLowPassFrequency() const
{
	return mActualLowPassFrequency;
}

float StftFilter::getActualHighPassFrequency() const
{
	return mActualHighPassFrequency;
}

int StftFilter::getMagnitudeIndexStart() const
{
	return mMagnitudeIndexStart;
}

int StftFilter::getMagnitudeIndexEnd() const
{
	return mMagnitudeIndexEnd;
}

namespace {
const static std::string VIEWABLE_BINS_KEY("Viewable FFT bins");
const static std::string LOW_PASS_FREQ_KEY("Low pass frequency (Hz)");
const static std::string HIGH_PASS_FREQ_KEY("High pass frequency (Hz)");
const static std::string CALCULATED_FFT_KEY("Calculated FFT size");
const static std::string ACTUAL_LP_FREQ_KEY("Calculated Low pass frequency (Hz)");
const static std::string ACTUAL_HP_FREQ_KEY("Calculated High pass frequency (Hz)");
}

void StftFilter::addToGui(ci::params::InterfaceGl* const gui)
{
	gui->addText("Filter parameters");
	gui->addParam<int>(VIEWABLE_BINS_KEY, [this](int val){ viewableBins(val); }, [this]()->int{ return getViewableBins(); });
	gui->addParam<float>(LOW_PASS_FREQ_KEY, [this](float val){ lowPassFrequency(val); }, [this]()->float{ return getLowPassFrequency(); });
	gui->addParam<float>(HIGH_PASS_FREQ_KEY, [this](float val){ highPassFrequency(val); }, [this]()->float{ return getHighPassFrequency(); });

	gui->addParam(CALCULATED_FFT_KEY, &mCalculatedFftSize, "readonly=true");
	gui->addParam(ACTUAL_LP_FREQ_KEY, &mLowPassFrequency, "readonly=true");
	gui->addParam(ACTUAL_HP_FREQ_KEY, &mHighPassFrequency, "readonly=true");

	gui->addSeparator();
}

void StftFilter::removeFromGui(ci::params::InterfaceGl* const gui)
{
	gui->setOptions(VIEWABLE_BINS_KEY, "readonly=true");
	gui->setOptions(LOW_PASS_FREQ_KEY, "readonly=true");
	gui->setOptions(HIGH_PASS_FREQ_KEY, "readonly=true");
}

}