#include "app_config.h"

#include <sstream>
#include <mutex>

#include <cinder/Json.h>

namespace cieq
{

AppConfig::AppConfig()
	: mConfigFile("stft.json", std::ios::in)
	, mRecordDuration(20.0f * 60.0f) // every half an hour, internals will reset
	, mTimeRange(20.0f) // 20 seconds
	, mWindowDuration(0.02f) // about 1024 samples in 20 seconds
	, mHopDuration(0.01f) // about 512 samples in 20 seconds
	, mFrequencyLow(100.0f) // 100Hz
	, mFrequencyHigh(10000.0f) // 100Hz
	, mSaturationLevelLow(0.0f)
	, mSaturationLevelHigh(1.0f)
	, mColorLow(ci::Color::white())
	, mColorHigh(ci::Color::black())
	, mRemoveStartButton(false)
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
			if (_tree.hasChild("frequency")) {
				if (_tree.hasChild("frequency.low")) {
					mFrequencyLow = _tree.getChild("frequency.low").getValue<float>();
				}
				if (_tree.hasChild("frequency.high")) {
					mFrequencyHigh = _tree.getChild("frequency.high").getValue<float>();
				}
			}
			if (_tree.hasChild("saturation_level")) {
				if (_tree.hasChild("saturation_level.low")) {
					mSaturationLevelLow = _tree.getChild("saturation_level.low").getValue<float>();
				}
				if (_tree.hasChild("saturation_level.high")) {
					mSaturationLevelHigh = _tree.getChild("saturation_level.high").getValue<float>();
				}
			}
			if (_tree.hasChild("color")) {
				if (_tree.hasChild("color.low")) {
					const auto r = _tree.hasChild("color.low.r") ? _tree.getChild("color.low.r").getValue<float>() : 0.0f;
					const auto g = _tree.hasChild("color.low.g") ? _tree.getChild("color.low.g").getValue<float>() : 0.0f;
					const auto b = _tree.hasChild("color.low.b") ? _tree.getChild("color.low.b").getValue<float>() : 0.0f;
					mColorLow = ci::Color(r, g, b);
				}
				if (_tree.hasChild("color.high")) {
					const auto r = _tree.hasChild("color.high.r") ? _tree.getChild("color.high.r").getValue<float>() : 0.0f;
					const auto g = _tree.hasChild("color.high.g") ? _tree.getChild("color.high.g").getValue<float>() : 0.0f;
					const auto b = _tree.hasChild("color.high.b") ? _tree.getChild("color.high.b").getValue<float>() : 0.0f;
					mColorHigh = ci::Color(r, g, b);
				}
			}
		}
		catch (...) { /*no op*/ }
	}

	checkSanity();
}

AppConfig::~AppConfig()
{
	if (mConfigFile) mConfigFile.close();
	mConfigFile.open("stft.json", std::ios::out);
	if (mConfigFile)
	{
		mConfigFile << generateConfig() << std::endl;
	}
}

std::string AppConfig::generateConfig() const
{
	std::stringstream buf;

	buf << "{" << std::endl;

	buf << "\t" << "\"record_duration\" : " << mRecordDuration << "," << std::endl;
	buf << "\t" << "\"time_range\" : " << mTimeRange << "," << std::endl;
	buf << "\t" << "\"window_duration\" : " << mWindowDuration << "," << std::endl;
	buf << "\t" << "\"hop_duration\" : " << mHopDuration << "," << std::endl;
	buf << "\t" << "\"frequency\" : { \"low\" : " << mFrequencyLow << ", \"high\" : " << mFrequencyHigh << " }" << "," << std::endl;
	buf << "\t" << "\"saturation_level\" : { \"low\" : " << mSaturationLevelLow << ", \"high\" : " << mSaturationLevelHigh << " }" << "," << std::endl;
	buf << "\t" << "\"color\" : {" << std::endl
		<< "\t\t\"low\" : { \"r\" : " << mColorLow.r << ", \"g\" : " << mColorLow.g << ", \"b\" : " << mColorLow.b << " }," << std::endl
		<< "\t\t\"high\" : { \"r\" : " << mColorHigh.r << ", \"g\" : " << mColorHigh.g << ", \"b\" : " << mColorHigh.b << " }"
		<< std::endl << "\t}" << std::endl;

	buf << "}";

	return buf.str();
}

void AppConfig::checkSanity()
{
	if (mTimeRange > mRecordDuration) mTimeRange = mRecordDuration;
	if (mWindowDuration > mTimeRange) mWindowDuration = mTimeRange;
	if (mHopDuration > mWindowDuration) mHopDuration = mWindowDuration;
	if (mFrequencyLow < 0) mFrequencyLow = 0;
	if (mFrequencyHigh > 25000.0f) mFrequencyHigh = 25000.0f;
}

void AppConfig::setRemoveLaunchParams()
{
	static std::once_flag _remove_it;
	std::call_once(_remove_it, [this]{ mRemoveStartButton = true; });
}

bool AppConfig::shouldRemoveLaunchParams() const
{
	return mRemoveStartButton;
}

void AppConfig::LaunchParamsRemoved()
{
	if (shouldRemoveLaunchParams())
	{
		static std::once_flag _removed;
		std::call_once(_removed, [this]{ mRemoveStartButton = false; });
	}
}

AudioNodes::Format AppConfig::getAsNodeFromat()
{
	checkSanity();

	AudioNodes::Format fmt;
	
	fmt
		.fftBins(2048) //fix me
		.windowDuration(mWindowDuration)
		.hopDuration(mHopDuration)
		.autoStart(true)
		.recordDuration(mRecordDuration)
		.samplesCacheSize(50) //to do: can we smartly guess this?
		.timeSpan(mTimeRange);

	return fmt;
}

} //!cieq