#ifndef CIEQ_INCLUDE_APP_GLOBALS_H_
#define CIEQ_INCLUDE_APP_GLOBALS_H_

#include <memory>

namespace cinder {
namespace audio {
class Context;
} //!namespace audio
namespace params {
class InterfaceGl;
}}//!namespace ci

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
	void								setParamsPtr(cinder::params::InterfaceGl* const params);
	cinder::params::InterfaceGl* const	getParamsPtr();
	void								setThreadRenderer(ThreadRenderer* const);
	ThreadRenderer* const				getThreadRenderer();
	void								setDrawContiguous(bool on);
	bool								getDrawContiguous() const;

private:
	AppEvent&							mEventProcessor;
	work::Manager&						mWorkManager;
	AudioNodes&							mAudioNodes;
	ThreadRenderer*						mThreadRenderer;
	cinder::params::InterfaceGl*		mParamsPtr;
	bool								mDrawContiguous;
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_GLOBALS_H_