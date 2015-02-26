#include "recorder_node.h"

#include <cinder/audio/Context.h>

namespace cieq {
namespace audio {

RecorderNode::RecorderNode(const Format &format /*= Format()*/)
	: inherited(format)
	, mWindowSize(format.getWindowSize())
	, mHopSize(format.getHopSize())
	, mLastQueried(0)
	, mCanQuery(true)
{}

RecorderNode::RecorderNode(size_t numFrames, const Format &format /*= Format()*/)
	: inherited(numFrames, format)
	, mWindowSize(format.getWindowSize())
	, mHopSize(format.getHopSize())
	, mLastQueried(0)
	, mCanQuery(true)
{}

RecorderNode::RecorderNode(float numSeconds, const Format &format /*= Format()*/)
	: inherited(static_cast<size_t>(ci::audio::Context::master()->getSampleRate() * numSeconds), format)
	, mWindowSize(format.getWindowSize())
	, mHopSize(format.getHopSize())
	, mLastQueried(0)
	, mCanQuery(true)
{}

ci::audio::BufferDynamic& RecorderNode::getBufferRaw()
{
	return mRecorderBuffer;
}

void RecorderNode::getBufferChunk(size_t start, size_t len, ci::audio::Buffer& other)
{
	getBufferRaw().copyOffset(other, len, start, 0);
}

void RecorderNode::getBufferChunk(size_t start, ci::audio::Buffer& other)
{
	getBufferChunk(start, mWindowSize, other);
}

bool RecorderNode::popBufferWindow(ci::audio::Buffer& other)
{
	if (!canQuery()) return false;

	if (mLastQueried + mWindowSize <= getWritePosition())
	{
		getBufferChunk(mLastQueried, other);
		mLastQueried += mHopSize;
		return true;
	}
	// This is the critical situation that can happen at the end of samples
	// where there's not enough number of samples for the last buffer.
	else if (getNumFrames() - mLastQueried < mWindowSize)
	{
		getBufferChunk(mLastQueried, getNumFrames() - mLastQueried, other);
		mLastQueried = getNumFrames();
		mCanQuery = false;
		return true;
	}
	else
	{
		return false;
	}
}

size_t RecorderNode::getWindowSize() const
{
	return mWindowSize;
}

size_t RecorderNode::getHopSize() const
{
	return mHopSize;
}

bool RecorderNode::canQuery() const
{
	return mCanQuery;
}

}} //!cieq::node