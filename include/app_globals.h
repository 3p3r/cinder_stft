#ifndef CIEQ_INCLUDE_APP_GLOBALS_H_
#define CIEQ_INCLUDE_APP_GLOBALS_H_

#include <memory>

namespace cinder {
namespace audio {
class Context;
}} //!namespace ci::audio

namespace cieq
{
class ThreadRenderer;
namespace work {
class Manager;
}; //!cieq::work
class AudioNodes;
class AppEvent;

/*!
 * \class AppGlobals
 * \brief The class that holds app-wide variables. Acts
 * like an storage for globally available objects.
 */
class AppGlobals
{
public:
	AppGlobals(AppEvent&, work::Manager&, AudioNodes&);

	AppEvent&							getEventProcessor();
	work::Manager&						getWorkManager();
	AudioNodes&							getAudioNodes();
	cinder::audio::Context&				getAudioContext();
	void								setThreadRenderer(ThreadRenderer* const);
	ThreadRenderer* const				getThreadRenderer();

private:
	AppEvent&							mEventProcessor;
	work::Manager&						mWorkManager;
	AudioNodes&							mAudioNodes;
	ThreadRenderer*						mThreadRenderer;
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_GLOBALS_H_