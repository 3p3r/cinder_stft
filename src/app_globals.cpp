#include "app_globals.h"
#include "app_event.h"

#include <cinder/audio/Context.h>

namespace cieq
{

AppGlobals::AppGlobals(AppEvent& event_processor)
	: mEventProcessor(event_processor)
	, mDrawContiguous(false)
{
	getEventProcessor().addKeyboardEvent([this](int k) {
		if (k == 'c' || k == 'C')
		{
			if (getDrawContiguous())
			{
				setDrawContiguous(false);
			}
			else
			{
				setDrawContiguous(true);
			}
		}
	});
}

AppEvent& AppGlobals::getEventProcessor()
{
	return mEventProcessor;
}

cinder::audio::Context& AppGlobals::getAudioContext()
{
	return *ci::audio::Context::master();
}

void AppGlobals::setParamsPtr(ci::params::InterfaceGl* const params)
{
	mParamsPtr = params;
}

ci::params::InterfaceGl* const AppGlobals::getParamsPtr()
{
	return mParamsPtr;
}

void AppGlobals::setDrawContiguous(bool on)
{
	mDrawContiguous = on;
}

bool AppGlobals::getDrawContiguous() const
{
	return mDrawContiguous;
}

} // !namespace cieq