#ifndef CIEQ_INCLUDE_APP_CONFIG_H_
#define CIEQ_INCLUDE_APP_CONFIG_H_

#include <string>
#include <fstream>

#include <Cinder/Color.h>

#include "audio_nodes.h"

namespace cinder {
namespace params {
class InterfaceGl;
}} //!ci::params

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
	float			mCutoffFrequency;
	int				mGuaranteedBins;
	int				mFftSize;

	void			setRemoveLaunchParams();
	void			LaunchParamsRemoved();
	bool			shouldRemoveLaunchParams() const;
	AudioNodes::Format
					getAsNodeFromat();

	void			addToGui(cinder::params::InterfaceGl* const);
	void			removeFromGui(cinder::params::InterfaceGl* const);

private:
	std::string		generateConfig() const;
	void			checkSanity();
	std::fstream	mConfigFile;
	bool			mRemoveStartButton;
};

} // !namespace cieq

#endif // !CIEQ_INCLUDE_APP_CONFIG_H_