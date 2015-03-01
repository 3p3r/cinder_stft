#include "stft_client_storage.h"

#include <cinder/CinderMath.h>

namespace cieq {
namespace stft {

ClientStorage::ClientStorage(const Client::Format& fmt)
	: mFftSize(fmt.getFftSize())
	, mWindowType(fmt.getWindowType())
	, mWindowSize(fmt.getWindowSize())
	, mChannelSize(fmt.getChannelSize())
	, mSmoothingFactor(0.5f)
{
	if (mFftSize < mWindowSize)
		mFftSize = mWindowSize;
	if (!ci::isPowerOf2(mFftSize))
		mFftSize = ci::nextPowerOf2(static_cast<uint32_t>(mFftSize));

	mFft = std::make_unique<ci::audio::dsp::Fft>(mFftSize);
	mFftBuffer = ci::audio::Buffer(mFftSize, mChannelSize);
	mCopiedBuffer = ci::audio::Buffer(mFftSize, mChannelSize);
	mBufferSpectral = ci::audio::BufferSpectral(mFftSize);
	mMagSpectrum.resize(mFftSize / 2);

	if (!mWindowSize)
		mWindowSize = mFftSize;
	else if (!ci::isPowerOf2(mWindowSize))
		mWindowSize = ci::nextPowerOf2(static_cast<uint32_t>(mWindowSize));

	mWindowingTable = ci::audio::makeAlignedArray<float>(mWindowSize);
	generateWindow(mWindowType, mWindowingTable.get(), mWindowSize);
}

}} //!cieq::stft