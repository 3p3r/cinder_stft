#include "audio_nodes.h"
#include "app_globals.h"
#include "app_event.h"
#include "recorder_node.h"
#include "stft_client.h"
#include "stft_request.h"
#include "smart_surface.h"

#include <cinder/audio/Context.h>
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

	const auto duration = 20.0f; //in seconds
	auto recorderFormat = cieq::audio::RecorderNode::Format().hopSize(100);
	mBufferRecorderNode = mGlobals.getAudioContext().makeNode(new cieq::audio::RecorderNode(duration, recorderFormat));

	mInputDeviceNode >> mBufferRecorderNode;

	mBufferRecorderNode->start();

	auto fmt = stft::Client::Format();
	fmt.channels(mBufferRecorderNode->getNumChannels()).fftSize(2048).windowSize(1024);
	mStftClient = work::make_client<stft::Client>(mGlobals.getWorkManager(), &mGlobals, fmt);

	mThreadRenderer = std::make_unique<ThreadRenderer>(*this, 400, 2048 / 2);
	mGlobals.setThreadRenderer(mThreadRenderer.get());

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

	mThreadRenderer->update();

	std::size_t query_pos = 0;
	while (mBufferRecorderNode->popBufferWindow(query_pos))
	{
		mStftClient->request(work::make_request<stft::Request>(query_pos));
	}

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

cieq::audio::RecorderNode* const AudioNodes::getBufferRecorderNode()
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