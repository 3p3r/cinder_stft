#include "audio_recorder.h"
#include <fmod_errors.h>
#include <exception>
#include <thread>

namespace cieq
{

AudioRecorder::Options::Options()
	: mSampleRate(0)
	, mDuration(0)
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

int AudioRecorder::Options::getSampleRate() const
{
	return mSampleRate;
}

int AudioRecorder::Options::getDuration() const
{
	return mDuration;
}


AudioRecorder::AudioRecorder(const Options& opts /*= Options()*/)
	: mOpts(opts)
{
	/*
	Create a System object and initialize.
	*/
	mResult = FMOD::System_Create(&mSystem);
	checkFmodError(mResult);

	mResult = mSystem->getVersion(&mVersion);
	checkFmodError(mResult);

	if (mVersion < FMOD_VERSION)
	{
		throw std::exception("FMOD lib version doesn't match header version.");
	}

	mResult = mSystem->init(100, FMOD_INIT_NORMAL, nullptr);
	checkFmodError(mResult);

	mResult = mSystem->getRecordNumDrivers(&mRecordNumDrivers);
	checkFmodError(mResult);

	if (mRecordNumDrivers <= 0)
	{
		throw std::exception("No recording devices found/plugged in!  Aborting.");
	}

	mResult = mSystem->getRecordDriverInfo(0, NULL, 0, 0, &mRecordRate, 0, &mRecordChannels);
	checkFmodError(mResult);

	/*
	Create user sound to record into.
	*/
	memset(&mExInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	mExInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	mExInfo.numchannels = mRecordChannels;
	mExInfo.format = FMOD_SOUND_FORMAT_PCM16;
	mExInfo.defaultfrequency = mOpts.getSampleRate();
	mExInfo.length = mExInfo.defaultfrequency * sizeof(short) * mExInfo.numchannels * mOpts.getDuration();

	mResult = mSystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &mExInfo, &mSound);
	checkFmodError(mResult);

	mResult = mSystem->recordStart(0, mSound, false);
	checkFmodError(mResult);

	mResult = mSound->getLength(&mSoundLength, FMOD_TIMEUNIT_PCM);
	checkFmodError(mResult);

	/*bool is_recording = false;
	mSystem->isRecording(0, &is_recording);

	do
	{
		mResult = mSystem->update();
		checkFmodError(mResult);

		mSystem->isRecording(0, &is_recording);

		std::this_thread::sleep_for(std::chrono::milliseconds(10));

	} while (is_recording);*/
}

void AudioRecorder::checkFmodError(FMOD_RESULT result) const
{
	if (result != FMOD_OK)
	{
		throw std::exception(FMOD_ErrorString(result));
	}
}

AudioRecorder::~AudioRecorder()
{
	auto result = mSound->release();
	checkFmodError(result);
	result = mSystem->release();
	checkFmodError(result);
}

}