#include "app_globals.h"

namespace cieq
{

AppGlobals::AppGlobals(AppEvent& event_processor)
	: mEventProcessor(event_processor)
{}

AppEvent& AppGlobals::getEventProcessor()
{
	return mEventProcessor;
}

} // !namespace cieq