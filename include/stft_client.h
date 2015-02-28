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
	Client(work::Manager& m, Format fmt = Format());
	void							handle(work::RequestRef) override;

private:
	Format							mFormat;
};

}} // !namespace cieq::stft

#endif // !CIEQ_INCLUDE_STFT_CLIENT_H_