#include "stft_client.h"
#include "app_globals.h"
#include "audio_nodes.h"
#include "recorder_node.h"
#include "stft_request.h"
#include "stft_client_storage.h"

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
	ClientStorage*				mPrivateStorage;
} _resources;

static class ClientResourcesAllocator
{
public:
	void allocate(	const Client::Format& fmt,
					ClientResources& local_rsc)
	{
		std::lock_guard<std::mutex> _lock(mResourceLock);
		mPrivateMemory.push_back(std::make_unique<ClientStorage>(fmt));
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
		_resources_allocator.allocate( mFormat, _resources );
		_ready = true;
	}

	auto request_ptr = static_cast<stft::Request*>(req.get());
	mGlobals
		->getAudioNodes()
		.getBufferRecorderNode()
		->popBufferWindow(_resources.mPrivateStorage->mFftBuffer, request_ptr->getQueryPos());
	

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