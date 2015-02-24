#include "audio_nodes.h"
#include "app_globals.h"
#include "app_event.h"

#include <cinder/audio/Context.h>
#include <cinder/audio/MonitorNode.h>
#include <cinder/audio/SampleRecorderNode.h>
#include <cinder/app/App.h>

namespace cieq
{

AudioNodes::AudioNodes(AppGlobals& globals)
	: mGlobals(globals)
	, mIsEnabled(false)
	, mIsReady(false)
{}

void AudioNodes::setup(bool auto_enable /*= true*/)
{
	try
	{
		// Iterate through all devices on this machine and see if we can find any inputs.
		std::size_t num_inputs = 0;
		for (auto it = mGlobals.getAudioContext().deviceManager()->getDevices().cbegin()
			, end = mGlobals.getAudioContext().deviceManager()->getDevices().end(); it != end; ++it)
		{
			if ((*it)->getNumInputChannels() > 0) num_inputs++;
		}

		if (num_inputs == 0)
		{
			throw std::runtime_error("Could not find any input channels. Make sure your computer comes with a mic!");
		}

		mInputDeviceNode = mGlobals.getAudioContext().createInputDeviceNode();
	}
	catch (const std::exception& ex)
	{
		ci::app::getWindow()->setTitle(ci::app::getWindow()->getTitle() + " ( No audio input found. )");
		ci::app::console() << "no audio input found: " << ex.what() << std::endl;
		return;
	}
	catch (...)
	{
		ci::app::getWindow()->setTitle(ci::app::getWindow()->getTitle() + " ( Error occurred creating input node. )");
		ci::app::console() << "Unknown error occurred creating input node." << std::endl;
		return;
	}
	
	auto monitorFormat = ci::audio::MonitorNode::Format().windowSize(1024);
	mMonitorNode = mGlobals.getAudioContext().makeNode(new ci::audio::MonitorNode(monitorFormat));
	
	auto monitorSpectralFormat = ci::audio::MonitorSpectralNode::Format().fftSize(2048).windowSize(1024);
	mMonitorSpectralNode = mGlobals.getAudioContext().makeNode(new ci::audio::MonitorSpectralNode(monitorSpectralFormat));

	auto num_seconds = 20 * 60;
	auto sample_rate = mInputDeviceNode->getSampleRate();
	auto recorderFormat = ci::audio::BufferRecorderNode::Format();
	mBufferRecorderNode = mGlobals.getAudioContext().makeNode(new ci::audio::BufferRecorderNode(num_seconds * sample_rate));

	mInputDeviceNode >> mMonitorNode;
	mInputDeviceNode >> mBufferRecorderNode;
	mInputDeviceNode >> mMonitorSpectralNode;

	mBufferRecorderNode->start();

	mOriginalTitle = ci::app::getWindow()->getTitle();
	ci::app::getWindow()->setTitle(mOriginalTitle + " (" + mInputDeviceNode->getDevice()->getName() + ")");

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

	mIsReady = true;
}

void AudioNodes::update()
{
	if (!ready()) return;

	if (mBufferRecorderNode->getWritePosition() != mBufferRecorderNode->getNumFrames())
	{
		ci::app::getWindow()->setTitle(mOriginalTitle + " ( Recording... )");
	}
	else
	{
		ci::app::getWindow()->setTitle(mOriginalTitle + " (" + mInputDeviceNode->getDevice()->getName() + ")");
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

cinder::audio::BufferRecorderNode* const AudioNodes::getBufferRecorderNode()
{
	return mBufferRecorderNode.get();
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

bool AudioNodes::ready() const
{
	return mIsReady;
}

} //!cieq