#include "audio_nodes.h"
#include "app_globals.h"
#include "app_event.h"

#include <cinder/audio/Context.h>
#include <cinder/audio/MonitorNode.h>
#include <cinder/app/App.h>
#include "recorder_node.h"

namespace cieq
{

AudioNodes::AudioNodes(AppGlobals& globals)
	: mGlobals(globals)
	, mIsEnabled(false)
{}

void AudioNodes::setup(bool auto_enable /*= true*/)
{
	mInputDeviceNode = mGlobals.getAudioContext().createInputDeviceNode();
	
	auto monitorFormat = ci::audio::MonitorNode::Format().windowSize(1024);
	mMonitorNode = mGlobals.getAudioContext().makeNode(new ci::audio::MonitorNode(monitorFormat));
	
	auto monitorSpectralFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	mMonitorSpectralNode = mGlobals.getAudioContext().makeNode(new ci::audio::MonitorSpectralNode(monitorSpectralFormat));

	auto recorderFormat = cieq::audio::RecorderNode::Format();
	recorderFormat.setSampleRate(8000).setDuration(60).setAutoStart(true);
	mRecorderNode = mGlobals.getAudioContext().makeNode(new cieq::audio::RecorderNode(recorderFormat));

	mInputDeviceNode >> mMonitorNode;
	mInputDeviceNode >> mRecorderNode;
	mInputDeviceNode >> mMonitorSpectralNode;

	ci::app::getWindow()->setTitle(ci::app::getWindow()->getTitle() + " (" + mInputDeviceNode->getDevice()->getName() + ")");

	if (auto_enable)
	{
		enableInput();
	}

	mGlobals
		.getEventProcessor()
		.addKeyboardEvent([this](char c){ if (c == 's' || c == 'S') toggleInput(); });

	mGlobals
		.getEventProcessor()
		.addMouseEvent([this](float, float){ toggleInput(); });
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
	if (mIsEnabled) return;

	mGlobals.getAudioContext().enable();
	mInputDeviceNode->enable();

	mIsEnabled = true;
}

void AudioNodes::disableInput()
{
	if (!mIsEnabled) return;

	mGlobals.getAudioContext().disable();
	mInputDeviceNode->disable();

	mIsEnabled = false;
}

void AudioNodes::toggleInput()
{
	if (mIsEnabled)
	{
		disableInput();
	}
	else
	{
		enableInput();
	}
}

cieq::audio::RecorderNode* const AudioNodes::getRecorderNode()
{
	return mRecorderNode.get();
}

} //!cieq