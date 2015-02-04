#ifndef CIEQ_INCLUDE_APP_GLOBALS_H_
#define CIEQ_INCLUDE_APP_GLOBALS_H_

namespace cinder {
namespace audio {
class Context;
}} //!namespace ci::audio

namespace cieq
{

class AppEvent;

/*!
 * \class AppGlobals
 * \brief The class that holds app-wide variables. Acts
 * like an storage for globally available objects.
 */
class AppGlobals
{
public:
	AppGlobals(AppEvent&);

	AppEvent&						getEventProcessor();
	cinder::audio::Context&			getAudioContext();

private:
	AppEvent&						mEventProcessor;
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_GLOBALS_H_