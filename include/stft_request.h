#ifndef CIEQ_INCLUDE_STFT_REQUEST_H_
#define CIEQ_INCLUDE_STFT_REQUEST_H_

#include "work_request.h"

namespace cistft {
namespace stft {

class Request : public work::Request
{
public:
	Request(std::size_t query_pos) : mQueryPos(query_pos) {}
	std::size_t getQueryPos() const { return mQueryPos; }

private:
	std::size_t mQueryPos;
};

}} // !namespace cistft::stft

#endif // !CIEQ_INCLUDE_STFT_REQUEST_H_