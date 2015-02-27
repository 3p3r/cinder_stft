#ifndef CIEQ_INCLUDE_STFT_CLIENT_H_
#define CIEQ_INCLUDE_STFT_CLIENT_H_

#include "work_client.h"

#include <unordered_map>
#include <thread>

#include <cinder/audio/dsp/Fft.h>
#include <cinder/audio/Buffer.h>

namespace cieq {
namespace stft {

/*!
 * \class Client
 * \namespace cieq::stft
 * \brief a Client that will fork requests for performing STFT.
 * \note owns one Fft class and one WindowTable per thread.
 */
class Client : public work::Client
{
public:
	class Format
	{
	public:
		Format&			windowSize(std::size_t size);
		Format&			fftSize(std::size_t size);

		std::size_t		getWindowSize() const;
		std::size_t		getFftSize() const;

	private:
		std::size_t		mWindowSize;
		std::size_t		mFftSize;
	};

public:
	/*!
	 * \struct ClientResources
	 * \brief internal storage for a thread, therefore multiple
	 * threads running at the same time do not share an FFT session.
	 */
	struct ClientResources
		: boost::noncopyable
	{
		ClientResources(std::size_t fft_size, std::size_t window_size);
		ci::audio::dsp::Fft			mFftProcessor;
		ci::audio::AlignedArrayPtr	mWindowingTable;
	};
	//! Convenience typedef of ClientResources class
	typedef std::unique_ptr< ClientResources > ClientResourcesRef;

public:
	Client(work::Manager& m, Format fmt = Format());

	//! answers true if the current thread id has a resource attached.
	bool							hasResourcesForThisThread() const;
	//! constructs resources for the current thread.
	//! note safe to work without locks since one single thread calls it sequentially
	void							constructResourcesForThisThread();

private:
	Format							mFormat;
	std::unordered_map < std::thread::id, ClientResourcesRef >
									mResources;
};

}} // !namespace cieq::stft

#endif // !CIEQ_INCLUDE_STFT_CLIENT_H_