#ifndef CIEQ_INCLUDE_AUDIO_RECORDER_H_
#define CIEQ_INCLUDE_AUDIO_RECORDER_H_

#include "fmod.hpp"

namespace cieq
{

class AudioRecorder
{
public:
	class Options
	{
	public:
		Options();
		Options&	setSampleRate(int sample_rate);
		int			getSampleRate() const;
		Options&	setDuration(int sample_rate);
		int			getDuration() const;
	private:
		int mSampleRate;
		int mDuration;
	};

public:
	AudioRecorder(const Options& opts = Options());
	~AudioRecorder();

private:
	Options			mOpts;
	FMOD::System*	mSystem = 0;
	FMOD::Sound*	mSound = 0;
	FMOD_RESULT     mResult = FMOD_OK;
	unsigned int    mVersion = 0;
	unsigned int    mSoundLength = 0;
	unsigned int    mRecordPos = 0;
	int             mRecordRate = 0;
	int             mRecordChannels = 0;
	int             mRecordNumDrivers = 0;
	FMOD_CREATESOUNDEXINFO
					mExInfo;

private:
	void			checkFmodError(FMOD_RESULT result) const;
};

}

#endif //!CIEQ_INCLUDE_AUDIO_RECORDER_H_