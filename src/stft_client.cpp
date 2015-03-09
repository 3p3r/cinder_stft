#include "stft_client.h"
#include "app_globals.h"
#include "audio_nodes.h"
#include "recorder_node.h"
#include "stft_request.h"
#include "stft_client_storage.h"
#include "stft_renderer.h"

#include <cinder/audio/dsp/Fft.h>
#include <cinder/audio/Buffer.h>
#include <cinder/CinderMath.h>

#include <mutex>
#include <thread>

namespace cieq {
namespace stft {

namespace {

/*!
 * \struct ClientResources
 * \brief internal storage for a thread, therefore multiple
 * threads running at the same time do not share an FFT session.
 */
thread_local static struct ClientResources
{
	ClientStorage*				mPrivateStorage;
} _resources;

static class ClientResourcesAllocator
{
public:
	void allocate(	const Client::Format& fmt,
					AppGlobals* globals,
					ClientResources& local_rsc)
	{
		std::lock_guard<std::mutex> _lock(mResourceLock);
		mPrivateMemory.push_back(std::make_unique<ClientStorage>(fmt, globals));
		local_rsc.mPrivateStorage = mPrivateMemory.back().get();
	}

private:
	std::mutex	mResourceLock;
	/* mind: blown. */
	std::vector < std::unique_ptr < ClientStorage > >
				mPrivateMemory;

} _resources_allocator;

} //!namespace

Client::Client(work::Manager& m, AppGlobals* g /*= nullptr*/, Format fmt /*= Format()*/)
	: work::Client(m)
	, mFormat(fmt)
	, mGlobals(g)
{}

void Client::handle(work::RequestRef req)
{
	// Allocate once per thread.
	thread_local static bool _ready = false;
	if (!_ready)
	{
		// pass thread's local storage to the allocator function
		_resources_allocator.allocate( mFormat, mGlobals, _resources );
		_ready = true;
	}

	//! Receive the pointer from main thread that contains the audio data position to be processed
	auto request_ptr	= static_cast<stft::Request*>(req.get());
	//! Acquire the recorder pointer
	auto recorder_ptr	= mGlobals->getAudioNodes().getBufferRecorderNode();
	//! Acquire the renderer pointer
	auto& renderer_ref	= mGlobals->getThreadRenderer();

	//! Ask the recorder to return back number of samples with size of its window size
	recorder_ptr->queryBufferWindow(_resources.mPrivateStorage->mCopiedBuffer, request_ptr->getQueryPos());
	
	//! window the copied buffer and compute forward FFT transform
	//! For each channel, if more than one channel...
	if (_resources.mPrivateStorage->mChannelSize > 1)
	{
		// Make sure FFT buffer is all zeros
		_resources.mPrivateStorage->mFftBuffer.zero();
		// Naive average of all channels
		for (size_t ch = 0; ch < _resources.mPrivateStorage->mChannelSize; ch++)
		{
			for (size_t i = 0; i < _resources.mPrivateStorage->mWindowSize; i++)
			{
				_resources.mPrivateStorage->mFftBuffer[i] += _resources.mPrivateStorage->mCopiedBuffer.getChannel(ch)[i] * _resources.mPrivateStorage->mChannelScale;
			}
		}

		ci::audio::dsp::mul(	_resources.mPrivateStorage->mFftBuffer.getData(),
								_resources.mPrivateStorage->mWindowingTable.get(),
								_resources.mPrivateStorage->mFftBuffer.getData(),
								_resources.mPrivateStorage->mWindowSize);
	}
	else //! If one channel then...
	{
		_resources.mPrivateStorage->mFftBuffer.zero();
		ci::audio::dsp::mul(	_resources.mPrivateStorage->mCopiedBuffer.getData(),
								_resources.mPrivateStorage->mWindowingTable.get(),
								_resources.mPrivateStorage->mFftBuffer.getData(),
								_resources.mPrivateStorage->mWindowSize);
	}

	_resources.mPrivateStorage->mFft->forward(&_resources.mPrivateStorage->mFftBuffer, &_resources.mPrivateStorage->mBufferSpectral);

	float *real = _resources.mPrivateStorage->mBufferSpectral.getReal();
	float *imag = _resources.mPrivateStorage->mBufferSpectral.getImag();

	//! remove Nyquist component
	//! We don't exactly know what this is but it makes sense because at 0Hz, we're technically a flat line
	//! and therefore it does not make sense to have a phase shift. a non zero phase shift will produce wrong
	//! results for sqrt(re^2 + im^2)
	imag[0] = 0.0f;

	// compute normalized magnitude spectrum
	for (size_t i = 0; i < _resources.mPrivateStorage->mMagSpectrum.size(); i++)
	{
		const float& re = real[i];
		const float& im = imag[i];

		// apply a smooth transition from last value to new value
		_resources.mPrivateStorage->mMagSpectrum[i] =
			_resources.mPrivateStorage->mMagSpectrum[i] *
			_resources.mPrivateStorage->mSmoothingFactor +
			ci::math<float>::sqrt(re * re + im * im) *
			_resources.mPrivateStorage->mMagnitudeScale * (1 - _resources.mPrivateStorage->mSmoothingFactor);

		// no smoothing
		//_resources.mPrivateStorage->mMagSpectrum[i] = ci::math<float>::sqrt(re * re + im * im) * magScale;
	}

	const auto pos = request_ptr->getQueryPos();
	const auto surface_index = renderer_ref.getSurfaceIndexByQueryPos(pos);
	const auto index_in_surface = renderer_ref.getIndexInSurfaceByQueryPos(pos);

	renderer_ref.getSurface(surface_index, pos).fillRow(index_in_surface, _resources.mPrivateStorage->mMagSpectrum);
}

Client::Format& Client::Format::windowSize(std::size_t size)
{
	mWindowSize = size; return *this;
}

Client::Format& Client::Format::fftSize(std::size_t size)
{
	mFftSize = size; return *this;
}

std::size_t Client::Format::getWindowSize() const
{
	return mWindowSize;
}

std::size_t Client::Format::getFftSize() const
{
	return mFftSize;
}

Client::Format& Client::Format::channels(std::size_t size)
{
	mChannels = size;
	return *this;
}

std::size_t Client::Format::getChannelSize() const
{
	return mChannels;
}

Client::Format& Client::Format::windowType(ci::audio::dsp::WindowType type)
{
	mWindowType = type; return *this;
}

ci::audio::dsp::WindowType Client::Format::getWindowType() const
{
	return mWindowType;
}

}
} //!cieq::stft