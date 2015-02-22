#include "audio_recorder.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <stdexcept>
#include <thread>

namespace cieq
{

/*!
 * \struct FmodWrapper
 * \private
 * \brief A thin wrapper over the FMOD sound engine, designed for
 * audio recording via microphone / other physical inputs.
 * \note this class is not exposed publicly simply because the
 * entire FMOD functionality is not needed.
 */
struct FmodWrapper final : public std::enable_shared_from_this<FmodWrapper>
{
	//! \brief a C++ exception utility for throwing FMOD errors.
	//! \note provides construction by FMOD_RESULT.
	class FmodWrapperException final : public std::runtime_error
	{
	public:
		explicit FmodWrapperException(FMOD_RESULT code)
			: std::runtime_error(FMOD_ErrorString(code))
		{}
		explicit FmodWrapperException(const std::string& what)
			: std::runtime_error(what)
		{}
		explicit FmodWrapperException(const char* what)
			: std::runtime_error(what)
		{}
	};

	//! \brief A convenience for wrapping FMOD's ExInfo options in a class
	class FmodWrapperExInfoOptions
	{
		//! \note totally a convenience, don't want my line lengths exceed my text editor.
		using Self = FmodWrapperExInfoOptions;

	public:
		Self&				setFormat(FMOD_SOUND_FORMAT fmt) { mFormat = fmt; return *this; }
		Self&				setFrequency(int freq) { mFrequency = freq; return *this; }
		Self&				setLength(std::size_t len) { mLength = len; return *this; }

		FMOD_SOUND_FORMAT	getFormat() const { return mFormat; }
		int					getFrequency() const { return mFrequency; }
		std::size_t			getLength() const { return mLength; }

	private:
		FMOD_SOUND_FORMAT	mFormat{ FMOD_SOUND_FORMAT_PCMFLOAT };
		int					mFrequency{ 0 };
		std::size_t			mLength{ 0 };
	};

	//! \brief takes care of FMOD's system initialization and
	//! version checking stuff.
	FmodWrapper()
	{
		mResult = FMOD::System_Create(&mSystem);
		checkLastResult();

		mResult = mSystem->getVersion(&mVersion);
		checkLastResult();

		if (mVersion < FMOD_VERSION)
		{
			throw FmodWrapperException("FMOD lib version doesn't match header version.");
		}

		initializeSystem();
		getDriverInfo();
		zeroOutExInfo();
	}

	void					setup(const FmodWrapperExInfoOptions& opts)
	{
		setupExInfo(opts);
		setupSound();
	}

	void					startRecording()
	{
		if (!isRecording())
		{
			mResult = mSystem->recordStart(0, mSound, false);
			checkLastResult();
		}
	}

	void					stopRecording()
	{
		if (isRecording())
		{
			mResult = mSystem->recordStop(0);
			checkLastResult();
		}
	}

	bool					isRecording()
	{
		mSystem->isRecording(0, &mRecordStatus);
		return mRecordStatus;
	}

	void					update()
	{
		mResult = mSystem->update();
		checkLastResult();
	}

	~FmodWrapper()
	{
		if (mSound)
		{
			mResult = mSound->release();
			checkLastResult();
		}

		if (mSystem)
		{
			mResult = mSystem->release();
			checkLastResult();
		}
	}

private:
	void					checkLastResult() const
	{
		if (mResult != FMOD_OK)
		{
			throw FmodWrapperException(mResult);
		}
	}

	void					initializeSystem()
	{
		mResult = mSystem->init(100, FMOD_INIT_NORMAL, nullptr);
		checkLastResult();
	}

	void					setupExInfo(const FmodWrapperExInfoOptions& opts)
	{
		mExInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		mExInfo.numchannels = mRecordChannels;
		mExInfo.format = opts.getFormat();
		mExInfo.defaultfrequency = opts.getFrequency();
		mExInfo.length = mExInfo.defaultfrequency * sizeof(float) * mExInfo.numchannels * opts.getLength();
	}

	void					setupSound()
	{
		mResult = mSystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &mExInfo, &mSound);
		checkLastResult();

		mResult = mSound->getLength(&mSoundLength, FMOD_TIMEUNIT_PCM);
		checkLastResult();
	}

	void					getDriverInfo()
	{
		mResult = mSystem->getRecordNumDrivers(&mRecordNumDrivers);
		checkLastResult();

		if (mRecordNumDrivers <= 0)
		{
			throw FmodWrapperException("No recording devices found/plugged in!  Aborting.");
		}

		mResult = mSystem->getRecordDriverInfo(0, NULL, 0, 0, &mRecordRate, 0, &mRecordChannels);
		checkLastResult();
	}

	void					zeroOutExInfo()
	{
		std::memset(&mExInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	}

public:
	FMOD::System*			mSystem{ nullptr };
	FMOD::Sound*			mSound{ nullptr };
	FMOD_RESULT				mResult{ FMOD_OK };
	unsigned int			mVersion{ 0 };
	unsigned int			mSoundLength{ 0 };
	unsigned int			mRecordPos{ 0 };
	int						mRecordRate{ 0 };
	int						mRecordChannels{ 0 };
	int						mRecordNumDrivers{ 0 };
	bool					mRecordStatus{ false };
	FMOD_CREATESOUNDEXINFO	mExInfo;
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
	
	fmod_wrapper->setup(FmodWrapper::FmodWrapperExInfoOptions()
		.setFormat(FMOD_SOUND_FORMAT_PCMFLOAT)
		.setFrequency(opts.getSampleRate())
		.setLength(opts.getDuration()));

	if (mOpts.getAutoStart())
	{
		fmod_wrapper->startRecording();
	}

	mFmodWrapper = fmod_wrapper->shared_from_this();
}

}