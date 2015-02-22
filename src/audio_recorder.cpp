#include "audio_recorder.h"

#include <fmod.hpp>
#include <fmod_errors.h>

#include <stdexcept>

namespace cieq
{

template<FMOD_SOUND_FORMAT fmt>
/*!
* \struct SizeOfFmodDataType
* \brief a template-ized struct that returns size in bytes of a
* FMOD PCM sample datatype.
*/
struct SizeOfFmodDataType
{
	static const std::size_t value = 0;
};
template<>
struct SizeOfFmodDataType<FMOD_SOUND_FORMAT_PCMFLOAT>
{
	static const std::size_t value = sizeof(float);
};
template<>
struct SizeOfFmodDataType<FMOD_SOUND_FORMAT_PCM16>
{
	static const std::size_t value = sizeof(short);
};
template<>
struct SizeOfFmodDataType<FMOD_SOUND_FORMAT_PCM32>
{
	static const std::size_t value = sizeof(int);
};
/*!
 * \name getFmodTypeSize
 * \brief A utility that uses \a SizeOfFmodDataType to retrieve
 * the FMOD PCM sample size dynamically.
 * \see SizeOfFmodDataType
 */
std::size_t getFmodTypeSize(FMOD_SOUND_FORMAT fmt)
{
	switch (fmt)
	{
	case FMOD_SOUND_FORMAT_PCMFLOAT:
		return SizeOfFmodDataType<FMOD_SOUND_FORMAT_PCMFLOAT>::value;
	case FMOD_SOUND_FORMAT_PCM32:
		return SizeOfFmodDataType<FMOD_SOUND_FORMAT_PCM32>::value;
	case FMOD_SOUND_FORMAT_PCM16:
		return SizeOfFmodDataType<FMOD_SOUND_FORMAT_PCM16>::value;
	default:
		throw std::logic_error("FMOD type not implemented / recognized.");
		break;
	}
}

/*!
 * \class FmodWrapper
 * \private
 * \brief A thin wrapper over the FMOD sound engine, designed for
 * audio recording via microphone / other physical inputs.
 * \note this class is not exposed publicly simply because the
 * entire FMOD functionality is not needed.
 */
class FmodWrapper final : public std::enable_shared_from_this<FmodWrapper>
{
public:
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
		std::size_t			getFormatSize() const { return getFmodTypeSize(mFormat); }

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

	//! \brief sets up ExInfo struct for FMOD and initializes the sound pointer.
	void					setup(const FmodWrapperExInfoOptions& opts)
	{
		setupExInfo(opts);
		setupSound();
	}

	//! \brief Starts recording via microphone.
	void					startRecording()
	{
		if (!isRecording())
		{
			mResult = mSystem->recordStart(0, mSound, false);
			checkLastResult();
		}
	}

	//! \brief Stops recording via microphone.
	void					stopRecording()
	{
		if (isRecording())
		{
			mResult = mSystem->recordStop(0);
			checkLastResult();
		}
	}

	//! \brief Returns true if we're already recording.
	bool					isRecording()
	{
		mResult = mSystem->isRecording(0, &mRecordStatus);
		checkLastResult();
		return mRecordStatus;
	}

	//! \brief updates the FMOD engine for one tick.
	void					update()
	{
		mResult = mSystem->update();
		checkLastResult();
	}

	//! \brief deallocates and cleans up FMOD engine.
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
	//! \brief checks and THROWS if the last FMOD operation
	//! stored in mResult was not successful.
	void					checkLastResult() const
	{
		if (mResult != FMOD_OK)
		{
			throw FmodWrapperException(mResult);
		}
	}

	//! \brief initializes mSystem
	void					initializeSystem()
	{
		mResult = mSystem->init(100, FMOD_INIT_NORMAL, nullptr);
		checkLastResult();
	}

	//! \brief Sets up FMOD's ExInfo struct. Options class found above.
	void					setupExInfo(const FmodWrapperExInfoOptions& opts)
	{
		mExInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
		mExInfo.numchannels = mRecordChannels;
		mExInfo.format = opts.getFormat();
		mExInfo.defaultfrequency = opts.getFrequency();
		mExInfo.length = mExInfo.defaultfrequency * opts.getFormatSize() * mExInfo.numchannels * opts.getLength();
	}

	//! \brief sets up mSound pointer.
	void					setupSound()
	{
		mResult = mSystem->createSound(0, FMOD_LOOP_NORMAL | FMOD_OPENUSER, &mExInfo, &mSound);
		checkLastResult();

		mResult = mSound->getLength(&mSoundLength, FMOD_TIMEUNIT_PCM);
		checkLastResult();
	}

	//! \brief queries microphone driver information.
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

	//! Sets the ExInfo struct to all zeros.
	void					zeroOutExInfo()
	{
		std::memset(&mExInfo, 0, sizeof(FMOD_CREATESOUNDEXINFO));
	}

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

void AudioRecorder::start()
{
	mFmodWrapper->startRecording();
}

void AudioRecorder::pause()
{
	mFmodWrapper->stopRecording();
}

void AudioRecorder::update()
{
	mFmodWrapper->update();
}

bool AudioRecorder::isRecording()
{
	return mFmodWrapper->isRecording();
}

}