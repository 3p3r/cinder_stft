#include "app_globals.h"

#include <cinder/audio/Context.h>

namespace cieq
{

AppGlobals::AppGlobals(AppEvent& event_processor, work::Manager& work_manager, AudioNodes& nodes, StftRenderer& renderer)
	: mEventProcessor(event_processor)
	, mWorkManager(work_manager)
	, mAudioNodes(nodes)
	, mThreadRenderer(renderer)
{}

AppEvent& AppGlobals::getEventProcessor()
{
	return mEventProcessor;
}

cinder::audio::Context& AppGlobals::getAudioContext()
{
	return *ci::audio::Context::master();
}

work::Manager& AppGlobals::getWorkManager()
{
	return mWorkManager;
}

AudioNodes& AppGlobals::getAudioNodes()
{
	return mAudioNodes;
}

StftRenderer& AppGlobals::getThreadRenderer()
{
	return mThreadRenderer;
}

} // !namespace cieq