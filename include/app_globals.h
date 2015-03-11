#ifndef CISTFT_INCLUDE_APP_GLOBALS_H_
#define CISTFT_INCLUDE_APP_GLOBALS_H_

#include <memory>

namespace cinder {
namespace audio {
class Context;
}} //!namespace ci::audio

namespace cistft
{
class StftRenderer;
namespace work {
class Manager;
}; //!cistft::work
class AudioNodes;
class GridRenderer;
struct AppConfig;
class AppEvent;
class StftFilter;

/*!
 * \class AppGlobals
 * \brief The class that holds app-wide variables. Acts
 * like an storage for globally available objects.
 */
class AppGlobals
{
public:
	AppGlobals(	AppEvent&,
				work::Manager&,
				AudioNodes&,
				StftRenderer&,
				GridRenderer&,
				AppConfig&,
				StftFilter&);

	AppEvent&							getEventProcessor();
	work::Manager&						getWorkManager();
	AudioNodes&							getAudioNodes();
	cinder::audio::Context&				getAudioContext();
	StftRenderer&						getThreadRenderer();
	GridRenderer&						getGridRenderer();
	AppConfig&							getAppConfig();
	StftFilter&							getFilter();

private:
	AppEvent&							mEventProcessor;
	work::Manager&						mWorkManager;
	AudioNodes&							mAudioNodes;
	StftRenderer&						mThreadRenderer;
	GridRenderer&						mGridRenderer;
	AppConfig&							mAppConfig;
	StftFilter&							mFilter;
};

} //!cistft

#endif //!CISTFT_INCLUDE_APP_GLOBALS_H_