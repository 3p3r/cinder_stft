#include "app_globals.h"

#include <cinder/audio/Context.h>

namespace cieq
{

AppGlobals::AppGlobals(AppEvent& event_processor)
	: mEventProcessor(event_processor)
{}

AppEvent& AppGlobals::getEventProcessor()
{
	return mEventProcessor;
}

cinder::audio::Context& AppGlobals::getAudioContext()
{
	return *ci::audio::Context::master();
}

void AppGlobals::setParamsRef(const std::shared_ptr < ci::params::InterfaceGl >& params)
{
	mParamsRef = params;
}

ci::params::InterfaceGl* const AppGlobals::getParamsRef()
{
	return mParamsRef.get();
}

} // !namespace cieq