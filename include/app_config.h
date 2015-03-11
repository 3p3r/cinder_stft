#ifndef CISTFT_INCLUDE_APP_CONFIG_H_
#define CISTFT_INCLUDE_APP_CONFIG_H_

#include <string>
#include <fstream>

#include <Cinder/Color.h>

#include "audio_nodes.h"

namespace cinder {
namespace params {
class InterfaceGl;
}} //!ci::params

namespace cistft {

class AppConfig
{
public:
	AppConfig();
	~AppConfig();

	AppConfig&		recordDuration(float val);
	AppConfig&		timeRange(float val);
	AppConfig&		windowDuration(float val);
	AppConfig&		hopDuration(float val);
	AppConfig&		samplesCacheSize(int val);
	AppConfig&		minimumViewableBins(int val);
	AppConfig&		lowPassFrequency(float val);
	AppConfig&		highPassFrequency(float val);

	float			getRecordDuration() const;
	float			getTimeRange() const;
	float			getWindowDuration() const;
	float			getHopDuration() const;
	int				getSamplesCacheSize() const;
	int				getMinimumViewableBins() const;
	float			getLowPassFrequency() const;
	float			getHighPassFrequency() const;

	int				getActualViewableBins() const;
	float			getActualLowPassFrequency() const;
	float			getActualHighPassFrequency() const;

	int				getCalculatedFftSize() const;
	int				getMagnitudeIndexStart() const;
	int				getSampleRate() const;

	void			prepareLaunch();
	void			performLaunch();
	bool			shouldLaunch() const;

	int				getRecordDurationInSamples() const;
	int				getTimeSpanInSamples() const;
	int				getHopDurationInSamples() const;
	int				getWindowDurationInSamples() const;

	void			setupPreLaunchGUI(cinder::params::InterfaceGl* const);
	void			setupPostLaunchGUI(cinder::params::InterfaceGl* const);

	void			setup() const;

private:
	std::string		generateConfig() const;
	std::fstream	mConfigFile;
	bool			mPreparedForLaunch;

private:
	void			checkSanity();
	void			buildBandPass() const;
	void			checkDirty() const;

private:
	float			mRecordDuration;
	float			mTimeRange;
	float			mWindowDuration;
	float			mHopDuration;
	int				mMinimumViewableBins;
	float			mLowPassFrequency;
	float			mHighPassFrequency;

	mutable int		mSamplesCacheSize;
	mutable int		mActualViewableBins;
	mutable float	mActualLowPassFrequency;
	mutable float	mActualHighPassFrequency;
	mutable int		mCalculatedFftSize;
	mutable int		mMagnitudeIndexStart;
	mutable int		mSampleRate;
	mutable bool	mDirty;
};

} // !namespace cistft

#endif // !CISTFT_INCLUDE_APP_CONFIG_H_