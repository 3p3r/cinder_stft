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

	AppEvent&							getEventProcessor();
	cinder::audio::Context&				getAudioContext();
	void								setParamsRef(const std::shared_ptr < cinder::params::InterfaceGl >& params);
	cinder::params::InterfaceGl* const	getParamsRef();
private:
	AppEvent&							mEventProcessor;
	std::shared_ptr < cinder::params::InterfaceGl >
										mParamsRef;
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_GLOBALS_H_