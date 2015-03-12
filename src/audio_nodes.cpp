#include "audio_nodes.h"
#include "app_globals.h"
#include "app_config.h"
#include "recorder_node.h"
#include "stft_client.h"
#include "stft_request.h"
#include "grid_renderer.h"

#include <cinder/audio/Context.h>
#include <cinder/audio/MonitorNode.h>
#include <cinder/app/App.h>

namespace cistft
{

AudioNodes::AudioNodes(AppGlobals& globals)
	: mGlobals(globals)
	, mIsEnabled(false)
	, mIsInputReady(false)
	, mIsMonitorReady(false)
	, mIsRecorderReady(false)
	, mQueryPosition(0)
{}

void AudioNodes::setupInput()
{
	try
	{
		// Iterate through all devices on this machine and see if we can find any inputs.
		std::size_t num_inputs = 0;
		for (auto it = mGlobals.getAudioContext().deviceManager()->getDevices().cbegin()
			, end = mGlobals.getAudioContext().deviceManager()->getDevices().cend(); it != end; ++it)
		{
			if ((*it)->getNumInputChannels() > 0) num_inputs++;
		}

		if (num_inputs == 0)
		{
			ci::app::getWindow()->setTitle(ci::app::getWindow()->getTitle() + " ( Could not find any input channels. Make sure your computer comes with a mic! )");
			return;
		}

		mInputDeviceNode = mGlobals.getAudioContext().createInputDeviceNode();
	}
	catch (const std::exception&)
	{
		ci::app::getWindow()->setTitle(ci::app::getWindow()->getTitle() + " ( No audio input found. )");
		return;
	}
	catch (...)
	{
		ci::app::getWindow()->setTitle(ci::app::getWindow()->getTitle() + " ( Error occurred creating input node. )");
		return;
	}

	enableInput();
	mIsInputReady = true;
}

void AudioNodes::setupRecorder()
{
	if (!isInputReady()) return;

	mBufferRecorderNode = mGlobals.getAudioContext().makeNode(new cistft::audio::RecorderNode(mGlobals));
	mInputDeviceNode >> mBufferRecorderNode;

	auto stftClientFormat = stft::Client::Format()
		.channels(mBufferRecorderNode->getNumChannels())
		.fftSize(mGlobals.getAppConfig().getCalculatedFftSize())
		.windowSize(mGlobals.getAppConfig().getWindowDurationInSamples());

	mStftClient = work::make_client<stft::Client>(mGlobals.getWorkManager(), &mGlobals, stftClientFormat);

	mBufferRecorderNode->start();
	mIsRecorderReady = true;
}

void AudioNodes::setupMonitor()
{
	if (!isInputReady()) return;

	auto monitorFormat = ci::audio::MonitorNode::Format().windowSize(1024);
	mMonitorNode = mGlobals.getAudioContext().makeNode(new ci::audio::MonitorNode(monitorFormat));

	mInputDeviceNode >> mMonitorNode;

	mIsMonitorReady = true;
}

void AudioNodes::update()
{
	if (isRecorderReady())
	{
		while (mBufferRecorderNode->popBufferWindow(mQueryPosition))
		{
			mStftClient->request(work::make_request<stft::Request>(mQueryPosition));
		}

		auto _time_diff = getBufferRecorderNode()->getWritePosition() / static_cast<float>(getBufferRecorderNode()->getSampleRate()) - mGlobals.getAppConfig().getTimeRange();
		if (_time_diff < mGlobals.getAppConfig().getTimeRange()) _time_diff = 0.0f;

		mGlobals.getGridRenderer().setHorizontalBoundary(_time_diff, mGlobals.getAppConfig().getTimeRange() + _time_diff);

		if (!getBufferRecorderNode()->isRecording())
		{
			// set to loop audio recording forever.
			getBufferRecorderNode()->reset();
			getBufferRecorderNode()->start();
		}
	}
}

cistft::audio::RecorderNode* const AudioNodes::getBufferRecorderNode()
{
	return mBufferRecorderNode.get();
}

cinder::audio::MonitorNode* const AudioNodes::getMonitorNode()
{
	return mMonitorNode.get();
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

bool AudioNodes::isInputReady() const
{
	return mIsInputReady;
}

bool AudioNodes::isRecorderReady() const
{
	return mIsRecorderReady;
}

bool AudioNodes::isMonitorReady() const
{
	return mIsMonitorReady;
}

} //!cistft