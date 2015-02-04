#include "audio_nodes.h"
#include "app_globals.h"

#include <cinder/audio/Context.h>
#include <cinder/audio/MonitorNode.h>
#include <cinder/app/App.h>

namespace cieq
{

AudioNodes::AudioNodes(AppGlobals& globals)
	: mGlobals(globals)
{}

void AudioNodes::setup(bool auto_enable /*= true*/)
{
	mInputDeviceNode = mGlobals.getAudioContext().createInputDeviceNode();
	
	auto monitorFormat = ci::audio::MonitorNode::Format().windowSize(1024);
	mMonitorNode = mGlobals.getAudioContext().makeNode(new ci::audio::MonitorNode(monitorFormat));
	
	auto monitorSpectralFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	mMonitorSpectralNode = mGlobals.getAudioContext().makeNode(new ci::audio::MonitorSpectralNode(monitorSpectralFormat));

	mInputDeviceNode >> mMonitorNode;
	mInputDeviceNode >> mMonitorSpectralNode;

	ci::app::getWindow()->setTitle(ci::app::getWindow()->getTitle() + " (" + mInputDeviceNode->getDevice()->getName() + ")");

	if (auto_enable)
	{
		enableInput();
	}
}

cinder::audio::InputDeviceNode* const AudioNodes::getInputDeviceNode()
{
	return mInputDeviceNode.get();
}

cinder::audio::MonitorNode* const AudioNodes::getMonitorNode()
{
	return mMonitorNode.get();
}

cinder::audio::MonitorSpectralNode* const AudioNodes::getMonitorSpectralNode()
{
	return mMonitorSpectralNode.get();
}

void AudioNodes::enableInput()
{
	mGlobals.getAudioContext().enable();
	mInputDeviceNode->enable();
}

void AudioNodes::disableInput()
{
	mGlobals.getAudioContext().disable();
	mInputDeviceNode->disable();
}

} //!cieq