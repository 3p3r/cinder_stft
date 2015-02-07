#include "resampler_node.h"

#include <cinder/CinderMath.h>

#include <samplerate.h>

namespace cieq {
namespace audio {

ResamplerNode::ResamplerNode(const Format &format /*= Format()*/)
	: NodeAutoPullable(format)
	, mWindowSize(format.getWindowSize())
	, mTargetSampleRate(format.getTargetSampleRate())
{}

ResamplerNode::~ResamplerNode()
{}

void ResamplerNode::initialize()
{
	if (!mWindowSize)
		mWindowSize = getFramesPerBlock();
	else if (!ci::isPowerOf2(mWindowSize))
		mWindowSize = ci::nextPowerOf2(static_cast<uint32_t>(mWindowSize));

	if (!mTargetSampleRate)
		mTargetSampleRate = getFramesPerBlock();

	mCopiedBuffer = ci::audio::Buffer(mWindowSize, getNumChannels());

	for (std::size_t ch = 0; ch < getNumChannels(); ch++) {
		mSrcDataVec.push_back(std::make_shared<SRC_DATA>());
	}
}

void ResamplerNode::process(ci::audio::Buffer *buffer)
{
	for (std::size_t ch = 0; ch < getNumChannels(); ch++) {
		mSrcDataVec[ch]->data_out = mCopiedBuffer.getChannel(ch);
		mSrcDataVec[ch]->output_frames = buffer->getNumFrames();
		mSrcDataVec[ch]->src_ratio = mTargetSampleRate / 44100.0f; //fix me
		mSrcDataVec[ch]->data_in = buffer->getChannel(ch);
		mSrcDataVec[ch]->input_frames = buffer->getNumFrames();
		src_simple(mSrcDataVec[ch].get(), SRC_SINC_BEST_QUALITY, 1);
	}
}

const ci::audio::Buffer& ResamplerNode::getBuffer()
{
	return mCopiedBuffer;
}

}
} //!cieq::audio