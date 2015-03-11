#include "recorder_node.h"

#include <cinder/audio/Context.h>

namespace cistft {
namespace audio {

RecorderNode::RecorderNode(size_t numFrames, const Format &format /*= Format()*/)
	: inherited(numFrames, format)
	, mWindowSize(format.getWindowSize())
	, mHopSize(format.getHopSize())
	, mLastQueried(0)
{
	mMaxPopsPossible = getNumFrames() / getHopSize();
}

ci::audio::BufferDynamic& RecorderNode::getBufferRaw()
{
	return mRecorderBuffer;
}

void RecorderNode::getBufferChunk(size_t start, size_t len, ci::audio::Buffer& other)
{
	other.copyOffset(getBufferRaw(), len, 0, start);
}

void RecorderNode::getBufferChunk(size_t start, ci::audio::Buffer& other)
{
	getBufferChunk(start, mWindowSize, other);
}

bool RecorderNode::popBufferWindow(size_t& query_pos)
{
	if (mLastQueried == getNumFrames())
	{
		return false;
	}
	else if (mLastQueried + mWindowSize <= getWritePosition())
	{
		query_pos = mLastQueried;
		mLastQueried += mHopSize;
		return true;
	}
	// This is the critical situation that can happen at the end of samples
	// where there's not enough number of samples for the last buffer.
	else if (getNumFrames() - mLastQueried < mWindowSize)
	{
		query_pos = mLastQueried;
		mLastQueried = getNumFrames();
		return true;
	}
	else
	{
		return false;
	}
}

void RecorderNode::queryBufferWindow(ci::audio::Buffer& other, size_t query_pos)
{
	if (query_pos + mWindowSize <= getWritePosition())
	{
		getBufferChunk(query_pos, other);
	}
	// This is the critical situation that can happen at the end of samples
	// where there's not enough number of samples for the last buffer.
	else if (getNumFrames() - query_pos < mWindowSize)
	{
		getBufferChunk(query_pos, getNumFrames() - query_pos, other);
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

bool RecorderNode::isRecording() const
{
	return getWritePosition() != getNumFrames();
}

size_t RecorderNode::getMaxPossiblePops() const
{
	return mMaxPopsPossible;
}

size_t RecorderNode::getQueryIndexByQueryPos(size_t pos)
{
	return pos / mHopSize;
}

void RecorderNode::reset()
{
	mLastQueried = 0;
}

}} //!cistft::node