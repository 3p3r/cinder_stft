#include "stft_client.h"

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
	ci::audio::dsp::Fft*		mFftProcessor;
	ci::audio::AlignedArrayPtr*	mWindowingTable;
} _resources;

static class ClientResourcesAllocator
{
public:
	void allocate(std::size_t fft_size, std::size_t window_size, ClientResources& local_rsc) {
		std::lock_guard<std::mutex> _lock(mResourceLock);
		mPrivateMemory.push_back(std::make_pair(ci::audio::dsp::Fft(fft_size), ci::audio::makeAlignedArray<float>(window_size)));
		local_rsc.mFftProcessor = &mPrivateMemory.back().first;
		local_rsc.mWindowingTable = &mPrivateMemory.back().second;
	}

private:
	std::mutex	mResourceLock;
	std::vector < std::pair< ci::audio::dsp::Fft, ci::audio::AlignedArrayPtr > >
				mPrivateMemory;
} _resources_allocator;

} //!namespace

Client::Client(work::Manager& m, Format fmt /*= Format()*/)
	: work::Client(m)
	, mFormat(fmt)
{}

void Client::handle(work::RequestRef)
{
	thread_local static bool _ready = false;
	if (!_ready)
	{
		// pass thread's local storage to the allocator function
		_resources_allocator.allocate(mFormat.getFftSize(), mFormat.getWindowSize(), _resources);
	}
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

}
} //!cieq::stft