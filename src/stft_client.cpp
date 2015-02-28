#include "stft_client.h"
#include "app_globals.h"
#include "audio_nodes.h"
#include "recorder_node.h"
#include "stft_request.h"

#include <cinder/audio/dsp/Fft.h>
#include <cinder/audio/Buffer.h>

#include <mutex>
#include <thread>
#include <tuple>

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
	ci::audio::dsp::Fft*		mFftProcessor;
	ci::audio::AlignedArrayPtr*	mWindowingTable;
	ci::audio::Buffer*			mAudioBuffer;
} _resources;

static class ClientResourcesAllocator
{
public:
	void allocate(	std::size_t fft_size,
					std::size_t window_size,
					std::size_t channel_size,
					ClientResources& local_rsc)
	{
		std::lock_guard<std::mutex> _lock(mResourceLock);
		
		mPrivateMemory.push_back(std::make_tuple(
			std::make_unique<ci::audio::dsp::Fft>(fft_size),
			ci::audio::makeAlignedArray<float>(window_size),
			std::make_shared<ci::audio::Buffer>(window_size, channel_size)));

		local_rsc.mFftProcessor		= std::get<0>(mPrivateMemory.back()).get();
		local_rsc.mWindowingTable	= &std::get<1>(mPrivateMemory.back());
		local_rsc.mAudioBuffer		= std::get<2>(mPrivateMemory.back()).get();
	}

private:
	std::mutex	mResourceLock;
	/* mind: blown. */
	std::vector < std::tuple <
		std::unique_ptr< ci::audio::dsp::Fft >,
		ci::audio::AlignedArrayPtr,
		ci::audio::BufferRef > >

				mPrivateMemory;
} _resources_allocator;

} //!namespace

Client::Client(work::Manager& m, Format fmt /*= Format()*/)
	: work::Client(m)
	, mFormat(fmt)
{}

void Client::handle(work::RequestRef req)
{
	// Allocate once per thread.
	thread_local static bool _ready = false;
	if (!_ready)
	{
		// pass thread's local storage to the allocator function
		_resources_allocator.allocate(
			mFormat.getFftSize(),
			mFormat.getWindowSize(),
			mFormat.getChannelSize(),
			_resources);
		_ready = true;
	}

	std::cout << "request for query pos: " << (static_cast<stft::Request*>(req.get())->getQueryPos()) << std::endl;
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

}
} //!cieq::stft