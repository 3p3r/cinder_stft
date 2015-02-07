#ifndef CIEQ_INCLUDE_RESAMPLER_NODE_H_
#define CIEQ_INCLUDE_RESAMPLER_NODE_H_

#include <cinder/audio/Node.h>
#include <cinder/audio/Context.h>
#include <cinder/audio/dsp/Dsp.h>
#include <cinder/audio/dsp/RingBuffer.h>

#include <samplerate.h>

namespace cieq {
namespace audio {


typedef std::shared_ptr<class ResamplerNode>			ResamplerNodeRef;
typedef std::shared_ptr<SRC_DATA>						LibSampleRateDataRef;

class ResamplerNode : public ci::audio::NodeAutoPullable
{
public:
	struct Format : public Node::Format {
		Format() : mWindowSize(0), mTargetSampleRate(0) {}

		//! Sets the window size, the number of samples that are recorded for one 'window' into the audio signal. Default is the Context's frames-per-block.
		//! \note will be rounded up to the nearest power of two.
		Format&		windowSize(std::size_t size)		{ mWindowSize = size; return *this; }
		//! Returns the window size.
		size_t		getWindowSize() const				{ return mWindowSize; }

		//! Sets the target re-sampling rate. Default is Context's default output sample rate
		Format&		targetSampleRate(std::size_t size)	{ mTargetSampleRate = size; return *this; }
		//! Returns the current target re-sampling rate.
		std::size_t	getTargetSampleRate() const			{ return mTargetSampleRate; }

		// reimpl Node::Format
		Format&		channels(size_t ch)					{ Node::Format::channels(ch); return *this; }
		Format&		channelMode(ChannelMode mode)		{ Node::Format::channelMode(mode); return *this; }
		Format&		autoEnable(bool autoEnable = true)	{ Node::Format::autoEnable(autoEnable); return *this; }

	protected:
		size_t mWindowSize;
		size_t mTargetSampleRate;
	};

public:
	ResamplerNode(const Format &format = Format());
	virtual ~ResamplerNode();

	//! Returns a filled Buffer of the sampled audio stream, suitable for consuming on the main UI thread.
	//! \note samples will only be copied if there is enough available in the internal dsp::RingBuffer.
	const ci::audio::Buffer& getBuffer();
	//! Returns the window size, which is the number of samples that are copied from the audio stream. Equivalent to: \code getBuffer().size() \endcode.
	size_t getWindowSize() const	{ return mWindowSize; }

protected:
	void initialize()						override;
	void process(ci::audio::Buffer *buffer)	override;

	ci::audio::Buffer					mCopiedBuffer;	// used to safely read audio frames on a non-audio thread
	std::size_t							mWindowSize;
	std::size_t							mRingBufferPaddingFactor;
	std::size_t							mTargetSampleRate;
	std::vector<LibSampleRateDataRef>	mSrcDataVec;
};

}} // !namespace cieq::audio

#endif // !CIEQ_INCLUDE_RESAMPLER_NODE_H_