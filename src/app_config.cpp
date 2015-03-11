#include "app_config.h"
#include "palette_manager.h"

#include <sstream>
#include <mutex>

#include <cinder/Json.h>
#include <cinder/params/Params.h>

#include <boost/algorithm/string/replace.hpp>

namespace cistft
{

namespace {
const std::string TEMPLATE("{\n\
	\"record_duration\":@RECORD_DURATION@,\n\
	\"time_range\":@TIME_RANGE@,\n\
	\"window_duration\":@WINDOW_DURATION@,\n\
	\"hop_duration\":@HOP_DURATION@,\n\
	\"frequency\":{\n\
		\"cutoff\":@FREQ_CUTOFF@,\n\
		\"bins\":@FREQ_BINS@\n\
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
	, mRecordDuration(20.0f * 60.0f) // every half an hour, internals will reset
	, mTimeRange(20.0f) // 20 seconds
	, mWindowDuration(0.02f) // about 1024 samples in 20 seconds
	, mHopDuration(0.01f) // about 512 samples in 20 seconds
	, mCutoffFrequency(22500) // 22.5KHz
	, mGuaranteedBins(256)
	, mFftSize(2048)
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
					mCutoffFrequency = _tree.getChild("frequency.cutoff").getValue<float>();
				}
				if (_tree.hasChild("frequency.high")) {
					mGuaranteedBins = _tree.getChild("frequency.bins").getValue<int>();
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
	boost::algorithm::replace_first(_template_copy, "@FREQ_CUTOFF@", std::to_string(mCutoffFrequency));
	boost::algorithm::replace_first(_template_copy, "@FREQ_BINS@", std::to_string(mGuaranteedBins));
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
	if (mCutoffFrequency < 0) mCutoffFrequency = 0;
	if (mGuaranteedBins < 0) mGuaranteedBins = 0;
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

namespace {
namespace GUI_STATICS {
const static std::string START_BUTTON("START");
const static std::string CONFIGURE_TEXT("Configure the parameters below and hit START");
const static std::string RECORD_TEXT("Record duration (s)");
const static std::string VIEWABLE_TEXT("Viewable Time range (s)");
const static std::string WINDOW_TEXT("Window duration (s)");
const static std::string HOP_TEXT("Hop duration (s)");
const static std::string CUTOFF_TEXT("Cutoff frequency (Hz)");
const static std::string BINS_TEXT("Guaranteed FFT bins");
}}

void AppConfig::addToGui(cinder::params::InterfaceGl* const gui)
{
	gui->addText(GUI_STATICS::CONFIGURE_TEXT);
	gui->addParam(GUI_STATICS::RECORD_TEXT, &mRecordDuration).min(300.0f).max(6000.0f).step(10.0f);
	gui->addParam(GUI_STATICS::VIEWABLE_TEXT, &mTimeRange).min(2.0f).max(20.0f).step(0.5f);
	gui->addParam(GUI_STATICS::WINDOW_TEXT, &mWindowDuration).min(0.01f).max(0.5f).step(0.01f);
	gui->addParam(GUI_STATICS::HOP_TEXT, &mHopDuration).min(0.005f).max(0.5f).step(0.005f);
	gui->addParam(GUI_STATICS::CUTOFF_TEXT, &mCutoffFrequency).min(0.0f).max(22500.0f).step(100.0f);
	gui->addParam(GUI_STATICS::BINS_TEXT, &mGuaranteedBins).min(0.0f).max(25000.0f).step(100.0f);
	gui->addSeparator();
	gui->addButton(GUI_STATICS::START_BUTTON, [this, gui] {
		gui->minimize();
		setRemoveLaunchParams();
	});
}

void AppConfig::removeFromGui(cinder::params::InterfaceGl* const gui)
{
	gui->removeParam(GUI_STATICS::START_BUTTON);
	gui->removeParam(GUI_STATICS::CONFIGURE_TEXT);
	gui->setOptions(GUI_STATICS::RECORD_TEXT, "readonly=true");
	gui->setOptions(GUI_STATICS::WINDOW_TEXT, "readonly=true");
	gui->setOptions(GUI_STATICS::HOP_TEXT, "readonly=true");
	gui->setOptions(GUI_STATICS::VIEWABLE_TEXT, "readonly=true");
	gui->setOptions(GUI_STATICS::BINS_TEXT, "readonly=true");
	gui->setOptions(GUI_STATICS::CUTOFF_TEXT, "readonly=true");
}

} //!cistft