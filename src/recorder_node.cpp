#include "recorder_node.h"

#include <cinder/audio/Context.h>

namespace cieq {
namespace audio {

RecorderNode::RecorderNode(const Format &format /*= Format()*/)
	: inherited(format)
	, mWindowSize(format.getWindowSize())
	, mHopSize(format.getHopSize())
	, mLastQueried(0)
{}

RecorderNode::RecorderNode(size_t numFrames, const Format &format /*= Format()*/)
	: inherited(numFrames, format)
	, mWindowSize(format.getWindowSize())
	, mHopSize(format.getHopSize())
	, mLastQueried(0)
{}

RecorderNode::RecorderNode(float numSeconds, const Format &format /*= Format()*/)
	: inherited(static_cast<size_t>(ci::audio::Context::master()->getSampleRate() * numSeconds), format)
	, mWindowSize(format.getWindowSize())
	, mHopSize(format.getHopSize())
	, mLastQueried(0)
{}

void RecorderNode::getBufferChunk(size_t start, size_t end, ci::audio::Buffer& other)
{
	getBufferRaw().copyOffset(other, mWindowSize, start, end);
}

ci::audio::BufferDynamic& RecorderNode::getBufferRaw()
{
	return mRecorderBuffer;
}

void RecorderNode::getBufferWindow(size_t start, ci::audio::Buffer& other)
{
	getBufferChunk(start, start + mWindowSize, other);
}

bool RecorderNode::popBufferWindow(ci::audio::Buffer& other)
{
	if (mLastQueried + mWindowSize >= getWritePosition())
	{
		getBufferWindow(mLastQueried, other);
		mLastQueried += mHopSize;
		return true;
	}
	else
	{
		return false;
	}
}

}} //!cieq::node