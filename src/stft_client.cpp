#include "stft_client.h"

namespace cieq {
namespace stft {


Client::ClientResources::ClientResources(std::size_t fft_size, std::size_t window_size)
	: mFftProcessor(fft_size)
	, mWindowingTable(ci::audio::makeAlignedArray<float>(window_size))
{}

Client::Client(work::Manager& m, Format fmt /*= Format()*/)
	: work::Client(m)
	, mFormat(fmt)
{}

bool Client::hasResourcesForThisThread() const
{
	return mResources.find(std::this_thread::get_id()) != mResources.end();
}

void Client::constructResourcesForThisThread()
{
	if (hasResourcesForThisThread()) return;
	mResources[std::this_thread::get_id()] = std::make_unique<ClientResources>(mFormat.getFftSize(), mFormat.getWindowSize());
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