#include "audio_recorder.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <exception>
#include <thread>

namespace cieq
{

struct FmodWrapper : public std::enable_shared_from_this<FmodWrapper>
{
	FMOD::System*			mSystem = 0;
	FMOD::Sound*			mSound = 0;
	FMOD_RESULT				mResult = FMOD_OK;
	unsigned int			mVersion = 0;
	unsigned int			mSoundLength = 0;
	unsigned int			mRecordPos = 0;
	int						mRecordRate = 0;
	int						mRecordChannels = 0;
	int						mRecordNumDrivers = 0;
	FMOD_CREATESOUNDEXINFO	mExInfo;
	void					checkFmodError(FMOD_RESULT result) const
	{
		if (result != FMOD_OK)
		{
			const std::string msg(FMOD_ErrorString(result));
			throw std::exception(msg.c_str());
		}
	}
	~FmodWrapper()
	{
		auto result = mSound->release();
		checkFmodError(result);
		result = mSystem->release();
		checkFmodError(result);
	}
};

AudioRecorder::Options::Options()
	: mSampleRate(0)
	, mDuration(0)
	, mAutoStart(false)
{}

AudioRecorder::Options& AudioRecorder::Options::setSampleRate(int sample_rate)
{
	if (mSampleRate != sample_rate)
		mSampleRate = sample_rate;
	return *this;
}

AudioRecorder::Options& AudioRecorder::Options::setDuration(int duration)
{
	if (mDuration != duration)
		mDuration = duration;
	return *this;
}

AudioRecorder::Options& AudioRecorder::Options::setAutoStart(bool on)
{
	if (mAutoStart != on)
		mAutoStart = on;
	return *this;
}

int AudioRecorder::Options::getSampleRate() const
{
	return mSampleRate;
}

int AudioRecorder::Options::getDuration() const
{
	return mDuration;
}

bool AudioRecorder::Options::getAutoStart() const
{
	return mAutoStart;
}


AudioRecorder::AudioRecorder(const Options& opts /*= Options()*/)
	: mOpts(opts)
{
	auto fmod_wrapper = std::make_shared<FmodWrapper>();
	/*
	Create a System object and initialize.
	*/
	fmod_wrapper->mResult = FMOD::System_Create(&fmod_wrapper->mSystem);
	fmod_wrapper->checkFmodError(fmod_wrapper->mResult);

	fmod_wrapper->mResult = fmod_wrapper->mSystem->getVersion(&fmod_wrapper->mVersion);
	fmod_wrapper->checkFmodError(fmod_wrapper->mResult);

	if (fmod_wrapper->mVersion < FMOD_VERSION)
	{
		throw std::exception("FMOD lib version doesn't match header version.");
	}

	fmod_wrapper->mResult = fmod_wrapper->mSystem->init(100, FMOD_INIT_NORMAL, nullptr);
	fmod_wrapper->checkFmodError(fmod_wrapper->mResult);

	fmod_wrapper->mResult = fmod_wrapper->mSystem->getRecordNumDrivers(&fmod_wrapper->mRecordNumDrivers);
	fmod_wrapper->checkFmodError(fmod_wrapper->mResult);

	if (fmod_wrapper->mRecordNumDrivers <= 0)
	{
		throw std::exception("No recording devices found/plugged in!  Aborting.");
	}

	fmod_wrapper->mResult = fmod_wrapper->mSystem->getRecordDriverInfo(0, NULL, 0, 0, &fmod_wrapper->mRecordRate, 0, &fmod_wrapper->mRecordChannels);
	fmod_wrapper->checkFmodError(fmod_wrapper->mResult);

	/*
	Create user sound to record into.
	*/
	memset(&fmod_wrapper->mExInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));

	fmod_wrapper->mExInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	fmod_wrapper->mExInfo.numchannels = fmod_wrapper->mRecordChannels;
	fmod_wrapper->mExInfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
	fmod_wrapper->mExInfo.defaultfrequency = mOpts.getSampleRate();
	fmod_wrapper->mExInfo.length = fmod_wrapper->mExInfo.defaultfrequency * sizeof(float) * fmod_wrapper->mExInfo.numchannels * mOpts.getDuration();

	fmod_wrapper->mResult = fmod_wrapper->mSystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &fmod_wrapper->mExInfo, &fmod_wrapper->mSound);
	fmod_wrapper->checkFmodError(fmod_wrapper->mResult);

	fmod_wrapper->mResult = fmod_wrapper->mSound->getLength(&fmod_wrapper->mSoundLength, FMOD_TIMEUNIT_PCM);
	fmod_wrapper->checkFmodError(fmod_wrapper->mResult);

	if (mOpts.getAutoStart())
	{
		fmod_wrapper->mResult = fmod_wrapper->mSystem->recordStart(0, fmod_wrapper->mSound, false);
		fmod_wrapper->checkFmodError(fmod_wrapper->mResult);
	}

	/*bool is_recording = false;
	mSystem->isRecording(0, &is_recording);

	do
	{
		mResult = mSystem->update();
		checkFmodError(mResult);

		mSystem->isRecording(0, &is_recording);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	} while (is_recording);*/

	mFmodWrapper = fmod_wrapper->shared_from_this();
}

}