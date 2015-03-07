#ifndef CIEQ_INCLUDE_APP_CONFIG_H_
#define CIEQ_INCLUDE_APP_CONFIG_H_

#include <string>
#include <fstream>

#include <Cinder/Color.h>

namespace cieq {

struct AppConfig
{
public:
	AppConfig();
	~AppConfig();

	float			mRecordDuration;
	float			mTimeRange;
	float			mWindowDuration;
	float			mHopDuration;
	float			mFrequencyLow;
	float			mFrequencyHigh;
	float			mSaturationLevelLow;
	float			mSaturationLevelHigh;
	ci::Color		mColorLow;
	ci::Color		mColorHigh;

	void			setRemoveLaunchParams();
	void			LaunchParamsRemoved();
	bool			shouldRemoveLaunchParams() const;

private:
	std::string		generateConfig() const;
	void			checkSanity();
	std::fstream	mConfigFile;
	bool			mRemoveStartButton;
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_APP_CONFIG_H_