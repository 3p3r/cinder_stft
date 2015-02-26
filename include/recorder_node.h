#ifndef CIEQ_INCLUDE_RECORDER_NODE_H_
#define CIEQ_INCLUDE_RECORDER_NODE_H_

#include <cinder/audio/SampleRecorderNode.h>

namespace cieq {
namespace audio {

class RecorderNode : public ci::audio::BufferRecorderNode
{
public:
	class Format : public ci::audio::BufferRecorderNode::Format
	{
	public:
		//! Sets the window size, the number of samples that are recorded for one 'window' into the audio processor thread.
		//! Default is 1024.
		//! \note will be rounded up to the nearest power of two. (via zero padding)
		Format&		windowSize(size_t size)		{ mWindowSize = size; return *this; }
		//! Sets the hop size, the number of samples to skip to get next window. default is 256 samples
		Format&		hopSize(size_t size)		{ mHopSize = size; return *this; }
		//! Returns the window size.
		size_t		getWindowSize() const		{ return mWindowSize; }
		//! Returns the hop size.
		size_t		getHopSize() const			{ return mHopSize; }

	protected:
		size_t		mWindowSize{ 1024 };
		size_t		mHopSize{ 256 };
	};

	RecorderNode(const Format &format = Format());
	//! Constructs a RecorderNode and initializes its buffer to be large enough to hold \a numFrames frames.
	RecorderNode(size_t numFrames, const Format &format = Format());
	//! Constructs a RecorderNode and initializes its buffer to be large enough to hold \a numSeconds frames.
	RecorderNode(float numSeconds, const Format &format = Format());

	ci::audio::BufferDynamic&		getBufferRaw();
	size_t							getWindowSize() const;
	size_t							getHopSize() const;
	void							getBufferChunk(size_t start, size_t len, ci::audio::Buffer& other);
	void							getBufferChunk(size_t start, ci::audio::Buffer& other);
	bool							popBufferWindow(ci::audio::Buffer& other);
	bool							canQuery() const;

protected:
	size_t	mWindowSize;
	size_t	mHopSize;
	size_t	mLastQueried;
	bool	mCanQuery;

private:
	using inherited = ci::audio::BufferRecorderNode;
};

}} // !namespace cieq

#endif // !CIEQ_INCLUDE_RECORDER_NODE_H_