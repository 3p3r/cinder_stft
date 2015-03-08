#include "audio_nodes.h"
#include "app_globals.h"
#include "app_event.h"
#include "recorder_node.h"
#include "stft_client.h"
#include "stft_request.h"

#include <cinder/audio/Context.h>
#include <cinder/app/App.h>

namespace cieq
{

AudioNodes::AudioNodes(AppGlobals& globals, const Format& fmt /*= Format()*/)
	: mGlobals(globals)
	, mFormat(fmt)
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

	if (mFormat.getAutoStart())
	{
		enableInput();
	}

	mGlobals
		.getEventProcessor()
		.addMouseCallback([this](float, float){ toggleInput(); });

	mIsInputReady = true;
}

void AudioNodes::setupRecorder()
{
	if (!mIsInputReady) return;

	auto recorderFormat = cieq::audio::RecorderNode::Format()
		.hopSize(mFormat.getHopDurationInSamples())
		.windowSize(mFormat.getWindowDurationInSamples());

	mBufferRecorderNode = mGlobals.getAudioContext().makeNode(new cieq::audio::RecorderNode(mFormat.getRecordDurationInSamples(), recorderFormat));
	mInputDeviceNode >> mBufferRecorderNode;

	auto stftClientFormat = stft::Client::Format()
		.channels(mBufferRecorderNode->getNumChannels())
		.fftSize(mFormat.getFftBins())
		.windowSize(mFormat.getWindowDurationInSamples());

	mStftClient = work::make_client<stft::Client>(mGlobals.getWorkManager(), &mGlobals, stftClientFormat);

	if (mFormat.getAutoStart())
	{
		mBufferRecorderNode->start();
	}

	mIsRecorderReady = true;
}

void AudioNodes::setupMonitor()
{
	if (!isRecorderReady()) return;
}

void AudioNodes::update()
{
	if (isRecorderReady())
	{
		while (mBufferRecorderNode->popBufferWindow(mQueryPosition))
		{
			mStftClient->request(work::make_request<stft::Request>(mQueryPosition));
		}
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

const AudioNodes::Format& AudioNodes::getFormat() const
{
	return mFormat;
}

// ------------------------------
// Audio nodes format
// ------------------------------

AudioNodes::Format::Format()
	: mRecordDuration(20.0f * 60.0f) //20 minutes
	, mTimeSpan(20.0f) //20 seconds
	, mWindowDuration(0.02f) //in seconds (0.02s is roughly 1024 in 44.1KHz)
	, mHopDuration(0.01f) //in seconds (0.01s roughly is 512 in 44.1KHz)
	, mFftBins(2048)
	, mSamplesCacheSize(50)
	, mAutoStart(true)
{}

float AudioNodes::Format::getRecordDuration() const
{
	return mRecordDuration;
}

AudioNodes::Format& AudioNodes::Format::recordDuration(float val)
{
	mRecordDuration = val; return *this;
}

float AudioNodes::Format::getTimeSpan() const
{
	return mTimeSpan;
}

AudioNodes::Format& AudioNodes::Format::timeSpan(float val)
{
	mTimeSpan = val; return *this;
}

float AudioNodes::Format::getWindowDuration() const
{
	return mWindowDuration;
}

AudioNodes::Format& AudioNodes::Format::windowDuration(float val)
{
	mWindowDuration = val; return *this;
}

float AudioNodes::Format::getHopDuration() const
{
	return mHopDuration;
}

AudioNodes::Format& AudioNodes::Format::hopDuration(float val)
{
	mHopDuration = val; return *this;
}

size_t AudioNodes::Format::getFftBins() const
{
	return mFftBins;
}

AudioNodes::Format& AudioNodes::Format::fftBins(size_t val)
{
	mFftBins = val; return *this;
}

bool AudioNodes::Format::getAutoStart() const
{
	return mAutoStart;
}

AudioNodes::Format& AudioNodes::Format::autoStart(bool val)
{
	mAutoStart = val; return *this;
}

size_t AudioNodes::Format::getSamplesCacheSize() const
{
	return mSamplesCacheSize;
}

AudioNodes::Format& AudioNodes::Format::samplesCacheSize(size_t val)
{
	mSamplesCacheSize = val; return *this;
}

namespace {
static size_t _get_sample_rate()
{
return ci::audio::Context::deviceManager()->getDefaultOutput()->getSampleRate();
}} //!namespace

size_t AudioNodes::Format::getRecordDurationInSamples() const
{
	return static_cast<std::size_t>(getRecordDuration() * _get_sample_rate());
}

size_t AudioNodes::Format::getTimeSpanInSamples() const
{
	return static_cast<std::size_t>(getTimeSpan() * _get_sample_rate());
}

size_t AudioNodes::Format::getHopDurationInSamples() const
{
	return static_cast<std::size_t>(getHopDuration() * _get_sample_rate());
}

size_t AudioNodes::Format::getWindowDurationInSamples() const
{
	return static_cast<std::size_t>(getWindowDuration() * _get_sample_rate());
}

} //!cieq