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
	// There's no point of having FFT size of less than a window size!
	if (mFftSize < mWindowSize)
		mFftSize = mWindowSize;

	// This makes sure that we are zero padding
	mFftSize = ci::nextPowerOf2(static_cast<uint32_t>(mFftSize));

	// The actual FFT processor instance
	mFft = std::make_unique<ci::audio::dsp::Fft>(mFftSize);

	// The FFT buffer, the one that will be filled AFTER FFT is performed on data
	mFftBuffer = ci::audio::Buffer(mFftSize, mChannelSize);

	// Copied buffer. the one that will be filled by raw audio chunks BEFORE FFT
	mCopiedBuffer = ci::audio::Buffer(mFftSize, mChannelSize);

	// Intermediate buffer passed to FFT processor
	mBufferSpectral = ci::audio::BufferSpectral(mFftSize);

	// The floating point array that contains all the FFT data, will be passed to renderer
	mMagSpectrum.resize(mFftSize / 2);

	// Window table.
	mWindowingTable = ci::audio::makeAlignedArray<float>(mWindowSize);
	generateWindow(mWindowType, mWindowingTable.get(), mWindowSize);
}

}} //!cieq::stft