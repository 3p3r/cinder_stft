#ifndef CIEQ_INCLUDE_AUDIO_RECORDER_H_
#define CIEQ_INCLUDE_AUDIO_RECORDER_H_

#include <memory>

namespace cieq
{

//! \note a shallow class forward declared here.
//! I do not need to expose the entire fmod.hpp
//! header through this class. Should be private.
class FmodWrapper;

/*!
 * \class AudioRecorder
 * \brief Records and audio stream from PC's default input
 * at a specified sample rate in Hz and duration in time.
 */
class  AudioRecorder
{
public:
	class Options
	{
	public:
		Options();
		Options&	setSampleRate(int sample_rate);
		Options&	setDuration(int duration);
		Options&	setAutoStart(bool on);
		
		int			getDuration() const;
		int			getSampleRate() const;
		bool		getAutoStart() const;

	private:
		int			mSampleRate;
		int			mDuration;
		bool		mAutoStart;
	};

public:
	AudioRecorder(const Options& opts = Options());
	void			start();
	void			pause();
	void			update();
	bool			isRecording();

private:
	//! aliasing my shallow forward declaration here.
	using FmodWrapperRef = std::shared_ptr < FmodWrapper >;

private:
	Options			mOpts;
	FmodWrapperRef	mFmodWrapper;
};

}

#endif //!CIEQ_INCLUDE_AUDIO_RECORDER_H_