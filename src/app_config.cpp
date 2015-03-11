#include "app_config.h"
#include "palette_manager.h"

#include <sstream>
#include <mutex>

#include <cinder/Json.h>
#include <cinder/params/Params.h>
#include <cinder/audio/Context.h>

#include <boost/algorithm/string/replace.hpp>

namespace cistft
{

namespace {
const std::string TEMPLATE("{\n\
	\"record_duration\":@RECORD_DURATION@,\n\
	\"time_range\":@TIME_RANGE@,\n\
	\"window_duration\":@WINDOW_DURATION@,\n\
	\"hop_duration\":@HOP_DURATION@,\n\
	\"viewable_bins\":@VIEWABLE_BINS@,\n\
	\"bandpass\":{\n\
		\"low_pass\":@FREQ_LOWPASS@,\n\
		\"high_pass\":@FREQ_HIGHPASS@\n\
	},\n\
	\"color_palette\":{\n\
		\"index\":@CP_INDEX@,\n\
		\"db_mode\":@CP_DB_MODE@,\n\
		\"db_mode_divisor\":@CP_DB_MODE_DIV@,\n\
		\"linear_mode_coeff\":@CP_LIN_MODE_COEFF@,\n\
		\"saturation_level\":{\n\
			\"low\":@SAT_LOW@,\n\
			\"high\":@SAT_HIGH@\n\
		}\n\
	}\n\
}");
const std::string CONFIG_FILENAME("stft.conf");
}

AppConfig::AppConfig()
	: mConfigFile(CONFIG_FILENAME, std::ios::in)
	, mRecordDuration(20.0f * 60.0f) // every 20 minutes, internals will reset
	, mTimeRange(20.0f) // 20 seconds in one screen
	, mWindowDuration(0.02f) // about 1024 samples in 20 seconds
	, mHopDuration(0.01f) // about 512 samples in 20 seconds
	, mMinimumViewableBins(256)
	, mLowPassFrequency(10000.0f) //10KHz
	, mHighPassFrequency(100.0f) //100Hz
	, mActualViewableBins(0)
	, mActualLowPassFrequency(0)
	, mActualHighPassFrequency(0)
	, mMagnitudeIndexStart(0)
	, mCalculatedFftSize(0)
	, mSampleRate(0)
	, mSamplesCacheSize(50)
	, mPreparedForLaunch(false)
	, mDirty(true)
{
	if (mConfigFile)
	{
		std::stringstream buf;
		buf << mConfigFile.rdbuf();
		try {
			ci::JsonTree _tree(buf.str());
			if (_tree.hasChild("record_duration")) {
				mRecordDuration = _tree.getChild("record_duration").getValue<float>();
			}
			if (_tree.hasChild("time_range")) {
				mTimeRange = _tree.getChild("time_range").getValue<float>();
			}
			if (_tree.hasChild("window_duration")) {
				mWindowDuration = _tree.getChild("window_duration").getValue<float>();
			}
			if (_tree.hasChild("hop_duration")) {
				mHopDuration = _tree.getChild("hop_duration").getValue<float>();
			}
			if (_tree.hasChild("viewable_bins")) {
				mMinimumViewableBins = _tree.getChild("viewable_bins").getValue<int>();
			}
			if (_tree.hasChild("bandpass"))
			{
				if (_tree.hasChild("bandpass.low_pass"))
				{
					mLowPassFrequency = _tree.getChild("bandpass.low_pass").getValue<float>();
				}
				if (_tree.hasChild("bandpass.high_pass"))
				{
					mHighPassFrequency = _tree.getChild("bandpass.high_pass").getValue<float>();
				}
			}
			if (_tree.hasChild("color_palette"))
			{
				if (_tree.hasChild("color_palette.saturation_level")) {
					if (_tree.hasChild("color_palette.saturation_level.low")) {
						palette::Manager::instance().setMinThreshold(_tree.getChild("color_palette.saturation_level.low").getValue<float>());
					}
					if (_tree.hasChild("color_palette.saturation_level.high")) {
						palette::Manager::instance().setMaxThreshold(_tree.getChild("color_palette.saturation_level.high").getValue<float>());
					}
				}

				if (_tree.hasChild("color_palette.index")) {
					palette::Manager::instance().setActivePalette(_tree.getChild("color_palette.index").getValue<int>());
				}

				if (_tree.hasChild("color_palette.db_mode")) {
					palette::Manager::instance().setConvertToDb(_tree.getChild("color_palette.db_mode").getValue<bool>());
				}

				if (_tree.hasChild("color_palette.db_mode_divisor")) {
					palette::Manager::instance().setDbDivisor(_tree.getChild("color_palette.db_mode_divisor").getValue<float>());
				}

				if (_tree.hasChild("color_palette.linear_mode_coeff")) {
					palette::Manager::instance().setLinearCoefficient(_tree.getChild("color_palette.linear_mode_coeff").getValue<float>());
				}
			}
		}
		catch (...) { /*no op*/ }
	}

	checkSanity();
}

namespace {
//! in terms of samples
static int MINIMUM_ZERO_PADDING_OFFSET = 128;
} //!namespace

void AppConfig::buildBandPass() const
{
	mCalculatedFftSize = static_cast<int>((mMinimumViewableBins * mSampleRate) / (mLowPassFrequency - mHighPassFrequency));

	if (!ci::isPowerOf2(mCalculatedFftSize))
		mCalculatedFftSize = ci::nextPowerOf2(mCalculatedFftSize);

	if (mCalculatedFftSize <= static_cast<int>(getWindowDurationInSamples()) + MINIMUM_ZERO_PADDING_OFFSET)
	{
		// The while loop guarantees we ALWAYS get zero padding
		while (mCalculatedFftSize <= static_cast<int>(getWindowDurationInSamples()))
		{
			mCalculatedFftSize = ci::nextPowerOf2(mCalculatedFftSize);
		}
	}

	// Less than 1024? get out of here.
	if (mCalculatedFftSize < 1024) mCalculatedFftSize = 1024;

	// a temporary place to calculate min / max frequency
	auto _temp_frequency = 0.0f;
	// index of operation placeholder
	auto index = 0;
	// Based on the calculated FFT size, calculate the frequency step per bin
	const auto _frequency_step = (1.0f / mCalculatedFftSize) * mSampleRate;

	// Let's find the high pass index first. This is where we start showing FFT data on screen
	for (index = 0; index < mCalculatedFftSize / 2 && _temp_frequency < mHighPassFrequency; ++index)
	{
		_temp_frequency += _frequency_step;
	}

	// double check if the above loop's operation is not passed beyond high pass frequency
	if (_temp_frequency > mHighPassFrequency)
	{
		// if so, subtract one from index and also go back one frequency step
		_temp_frequency -= _frequency_step;
		index--;
	}

	// We got the actual high pass frequency calculated here, in an ideal world, this is equal to what user has supplied
	mActualHighPassFrequency = _temp_frequency;
	// Get the index, we need it.
	mMagnitudeIndexStart = index;

	// Let's find the low pass index. This is where we finish showing FFT data on screen
	for (/* no op */; index < mCalculatedFftSize / 2 && _temp_frequency < mLowPassFrequency; ++index)
	{
		_temp_frequency += _frequency_step;
	}

	// double check if the above loop's operation is not passed beyond low pass frequency
	if (_temp_frequency > mLowPassFrequency)
	{
		// if so, subtract one from index and also go back one frequency step
		_temp_frequency -= _frequency_step;
		index--;
	}

	// Calculated the actual number of bins shown to the user
	mActualViewableBins = (index - mMagnitudeIndexStart) + 1;
	// We got the actual low pass frequency calculated here, in an ideal world, this is equal to what user has supplied
	mActualLowPassFrequency = _temp_frequency;
}

AppConfig::~AppConfig()
{
	if (mConfigFile) mConfigFile.close();
	mConfigFile.open(CONFIG_FILENAME, std::ios::out);
	if (mConfigFile)
	{
		mConfigFile << generateConfig() << std::endl;
	}
}

std::string AppConfig::generateConfig() const
{
	std::string _template_copy(TEMPLATE);

	boost::algorithm::replace_first(_template_copy, "@RECORD_DURATION@", std::to_string(mRecordDuration));
	boost::algorithm::replace_first(_template_copy, "@TIME_RANGE@", std::to_string(mTimeRange));
	boost::algorithm::replace_first(_template_copy, "@WINDOW_DURATION@", std::to_string(mWindowDuration));
	boost::algorithm::replace_first(_template_copy, "@HOP_DURATION@", std::to_string(mHopDuration));
	boost::algorithm::replace_first(_template_copy, "@VIEWABLE_BINS@", std::to_string(mMinimumViewableBins));
	boost::algorithm::replace_first(_template_copy, "@FREQ_LOWPASS@", std::to_string(mLowPassFrequency));
	boost::algorithm::replace_first(_template_copy, "@FREQ_HIGHPASS@", std::to_string(mHighPassFrequency));
	boost::algorithm::replace_first(_template_copy, "@CP_INDEX@", std::to_string(palette::Manager::instance().getActivePalette()));
	boost::algorithm::replace_first(_template_copy, "@CP_DB_MODE@", palette::Manager::instance().getConvertToDb() ? "true" : "false");
	boost::algorithm::replace_first(_template_copy, "@CP_DB_MODE_DIV@", std::to_string(palette::Manager::instance().getDbDivisor()));
	boost::algorithm::replace_first(_template_copy, "@CP_LIN_MODE_COEFF@", std::to_string(palette::Manager::instance().getLinearCoefficient()));
	boost::algorithm::replace_first(_template_copy, "@SAT_LOW@", std::to_string(palette::Manager::instance().getMinThreshold()));
	boost::algorithm::replace_first(_template_copy, "@SAT_HIGH@", std::to_string(palette::Manager::instance().getMaxThreshold()));

	return _template_copy;
}

void AppConfig::checkSanity()
{
	if (mTimeRange > mRecordDuration) mTimeRange = mRecordDuration;
	if (mWindowDuration > mTimeRange) mWindowDuration = mTimeRange;
	if (mHopDuration > mWindowDuration) mHopDuration = mWindowDuration;
	if (mHighPassFrequency < 0) mHighPassFrequency = 0;
	if (mHighPassFrequency > mLowPassFrequency) mHighPassFrequency = mLowPassFrequency;
	if (mMinimumViewableBins < 0) mMinimumViewableBins = 0;
}

void AppConfig::prepareLaunch()
{
	static std::once_flag _prepare_it;
	std::call_once(_prepare_it, [this]{ mPreparedForLaunch = true; });
}

bool AppConfig::shouldLaunch() const
{
	return mPreparedForLaunch;
}

void AppConfig::performLaunch()
{
	if (shouldLaunch())
	{
		static std::once_flag _launch;
		std::call_once(_launch, [this]{ mPreparedForLaunch = false; });
	}
}

AppConfig& AppConfig::minimumViewableBins(int val)
{
	mMinimumViewableBins = val;

	mDirty = true;
	return *this;
}

AppConfig& AppConfig::lowPassFrequency(float val)
{
	if (mSampleRate / 2.0f < val)
		val = mSampleRate / 2.0f;

	mLowPassFrequency = val;

	if (mHighPassFrequency > mLowPassFrequency)
		mLowPassFrequency = mHighPassFrequency;

	mDirty = true;
	return *this;
}

AppConfig& AppConfig::highPassFrequency(float val)
{
	if (val < 0.0f) val = 0.0f;

	mHighPassFrequency = val;

	if (mHighPassFrequency > mLowPassFrequency)
		mHighPassFrequency = mLowPassFrequency;

	mDirty = true;
	return *this;
}

AppConfig& AppConfig::recordDuration(float val)
{
	mRecordDuration = val;

	if (mRecordDuration < 0)
		mRecordDuration = 0;

	mDirty = true;
	return *this;
}

AppConfig& AppConfig::timeRange(float val)
{
	mTimeRange = val;

	if (mTimeRange < 0)
		mTimeRange = 0;

	mDirty = true;
	return *this;
}

AppConfig& AppConfig::windowDuration(float val)
{
	mWindowDuration = val;

	if (mWindowDuration < 0)
		mWindowDuration = 0;

	mDirty = true;
	return *this;
}

AppConfig& AppConfig::hopDuration(float val)
{
	mHopDuration = val;

	if (mHopDuration < 0)
		mHopDuration = 0;

	mDirty = true;
	return *this;
}

AppConfig& AppConfig::samplesCacheSize(int val)
{
	mSamplesCacheSize = val;

	if (mSamplesCacheSize < 0)
		mSamplesCacheSize = 0;

	return *this;
}

int AppConfig::getMinimumViewableBins() const
{
	checkDirty();
	return mMinimumViewableBins;
}

float AppConfig::getLowPassFrequency() const
{
	checkDirty();
	return mLowPassFrequency;
}

float AppConfig::getHighPassFrequency() const
{
	checkDirty();
	return mHighPassFrequency;
}

int AppConfig::getActualViewableBins() const
{
	checkDirty();
	return mActualViewableBins;
}

float AppConfig::getActualLowPassFrequency() const
{
	checkDirty();
	return mActualLowPassFrequency;
}

float AppConfig::getActualHighPassFrequency() const
{
	checkDirty();
	return mActualHighPassFrequency;
}

int AppConfig::getMagnitudeIndexStart() const
{
	checkDirty();
	return mMagnitudeIndexStart;
}

int AppConfig::getCalculatedFftSize() const
{
	checkDirty();
	return mCalculatedFftSize;
}

float AppConfig::getRecordDuration() const
{
	checkDirty();
	return mRecordDuration;
}

float AppConfig::getTimeRange() const
{
	checkDirty();
	return mTimeRange;
}

float AppConfig::getWindowDuration() const
{
	checkDirty();
	return mWindowDuration;
}

float AppConfig::getHopDuration() const
{
	checkDirty();
	return mHopDuration;
}

int AppConfig::getSamplesCacheSize() const
{
	checkDirty();
	return mSamplesCacheSize;
}

int AppConfig::getRecordDurationInSamples() const
{
	checkDirty();
	return static_cast<int>(getRecordDuration() * mSampleRate);
}

int AppConfig::getTimeSpanInSamples() const
{
	checkDirty();
	return static_cast<int>(getTimeRange() * mSampleRate);
}

int AppConfig::getHopDurationInSamples() const
{
	checkDirty();
	return static_cast<int>(getHopDuration() * mSampleRate);
}

int AppConfig::getWindowDurationInSamples() const
{
	checkDirty();
	return static_cast<int>(getWindowDuration() * mSampleRate);
}

int AppConfig::getSampleRate() const
{
	checkDirty();
	return mSampleRate;
}

void AppConfig::checkDirty() const
{
	if (mDirty) { setup(); }
}

namespace {
namespace GUI_STATICS {
const static std::string BINS_TEXT_KEY("Guaranteed FFT bins");
const static std::string LOW_PASS_FREQ_KEY("Low pass frequency (Hz)");
const static std::string HIGH_PASS_FREQ_KEY("High pass frequency (Hz)");
const static std::string CALCULATED_FFT_KEY("Calculated FFT size");
const static std::string ACTUAL_VIEWABLE_BINS_KEY("Calculated viewable bins");
const static std::string ACTUAL_LP_FREQ_KEY("Calculated Low pass frequency (Hz)");
const static std::string ACTUAL_HP_FREQ_KEY("Calculated High pass frequency (Hz)");
const static std::string CONFIGURE_TEXT_KEY("Configure the parameters below and hit START");
const static std::string RECORD_TEXT_KEY("Record duration (s)");
const static std::string VIEWABLE_TEXT_KEY("Viewable Time range (s)");
const static std::string WINDOW_TEXT_KEY("Window duration (s)");
const static std::string HOP_TEXT_KEY("Hop duration (s)");
const static std::string START_BUTTON_KEY("START");
}}

void AppConfig::addToGui(cinder::params::InterfaceGl* const gui)
{
	gui->addText("Filter parameters");
	gui->addParam<int>(GUI_STATICS::BINS_TEXT_KEY, [this](int val){ minimumViewableBins(val); }, [this]()->int{ return getMinimumViewableBins(); });
	gui->addParam<float>(GUI_STATICS::LOW_PASS_FREQ_KEY, [this](float val){ lowPassFrequency(val); }, [this]()->float{ return getLowPassFrequency(); });
	gui->addParam<float>(GUI_STATICS::HIGH_PASS_FREQ_KEY, [this](float val){ highPassFrequency(val); }, [this]()->float{ return getHighPassFrequency(); });

	gui->addParam(GUI_STATICS::CALCULATED_FFT_KEY, &mCalculatedFftSize, "readonly=true");
	gui->addParam(GUI_STATICS::ACTUAL_VIEWABLE_BINS_KEY, &mActualViewableBins, "readonly=true");
	gui->addParam(GUI_STATICS::ACTUAL_LP_FREQ_KEY, &mLowPassFrequency, "readonly=true");
	gui->addParam(GUI_STATICS::ACTUAL_HP_FREQ_KEY, &mHighPassFrequency, "readonly=true");
	gui->addSeparator();

	gui->addText(GUI_STATICS::CONFIGURE_TEXT_KEY);
	gui->addParam(GUI_STATICS::RECORD_TEXT_KEY, &mRecordDuration).min(300.0f).max(6000.0f).step(10.0f);
	gui->addParam(GUI_STATICS::VIEWABLE_TEXT_KEY, &mTimeRange).min(2.0f).max(20.0f).step(0.5f);
	gui->addParam(GUI_STATICS::WINDOW_TEXT_KEY, &mWindowDuration).min(0.01f).max(0.5f).step(0.01f);
	gui->addParam(GUI_STATICS::HOP_TEXT_KEY, &mHopDuration).min(0.0f).max(0.5f).step(0.005f);
	gui->addSeparator();

	gui->addButton(GUI_STATICS::START_BUTTON_KEY, [this, gui] {
		gui->minimize();
		prepareLaunch();
	});
}

void AppConfig::removeFromGui(cinder::params::InterfaceGl* const gui)
{
	gui->setOptions(GUI_STATICS::BINS_TEXT_KEY, "readonly=true");
	gui->setOptions(GUI_STATICS::LOW_PASS_FREQ_KEY, "readonly=true");
	gui->setOptions(GUI_STATICS::HIGH_PASS_FREQ_KEY, "readonly=true");

	gui->removeParam(GUI_STATICS::START_BUTTON_KEY);
	gui->removeParam(GUI_STATICS::CONFIGURE_TEXT_KEY);

	gui->setOptions(GUI_STATICS::RECORD_TEXT_KEY, "readonly=true");
	gui->setOptions(GUI_STATICS::WINDOW_TEXT_KEY, "readonly=true");
	gui->setOptions(GUI_STATICS::HOP_TEXT_KEY, "readonly=true");
	gui->setOptions(GUI_STATICS::VIEWABLE_TEXT_KEY, "readonly=true");
}

namespace {
static size_t _get_sample_rate()
{
return ci::audio::Context::deviceManager()->getDefaultOutput()->getSampleRate();
}} //!namespace

void AppConfig::setup() const
{
	if (!mDirty) return;
	
	mDirty = false;
	
	mSampleRate = _get_sample_rate();
	buildBandPass();
}

} //!cistft