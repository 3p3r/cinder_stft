#include "app_globals.h"

#include <cinder/audio/Context.h>

namespace cistft
{

AppGlobals::AppGlobals(work::Manager& work_manager, AudioNodes& nodes, StftRenderer& renderer, GridRenderer& grid_renderer, AppConfig& app_config)
	: mWorkManager(work_manager)
	, mAudioNodes(nodes)
	, mThreadRenderer(renderer)
	, mGridRenderer(grid_renderer)
	, mAppConfig(app_config)
{}

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

GridRenderer& AppGlobals::getGridRenderer()
{
	return mGridRenderer;
}

AppConfig& AppGlobals::getAppConfig()
{
	return mAppConfig;
}

} // !namespace cistft