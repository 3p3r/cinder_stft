#include "stft_client_storage.h"
#include "app_globals.h"
#include "app_config.h"

#include <cinder/CinderMath.h>

namespace cistft {
namespace stft {

ClientStorage::ClientStorage(const Client::Format& fmt, AppGlobals* const globals)
	: mFftSize(fmt.getFftSize())
	, mWindowType(fmt.getWindowType())
	, mWindowSize(fmt.getWindowSize())
	, mChannelSize(fmt.getChannelSize())
	, mSmoothingFactor(0.5f)
{
	// This makes sure that we are zero padding
	mFftSize = globals->getAppConfig().getCalculatedFftSize();

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

	// MISC.
	mMagnitudeScale = 1.0f / mFft->getSize();
	mChannelScale	= 1.0f / mChannelSize;
}

}} //!cistft::stft