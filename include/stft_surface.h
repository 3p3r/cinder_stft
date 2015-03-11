#ifndef CIEQ_INCLUDE_STFT_SURFACE_H_
#define CIEQ_INCLUDE_STFT_SURFACE_H_

#include <atomic>
#include <mutex>

#include <cinder/Surface.h>

namespace cistft {

class StftSurface final : public ci::Surface32f
{
public:
	StftSurface(int width, int height, int fft_vector_index);
	StftSurface() = delete;

	void				fillRow(int row, const std::vector<float>& data);
	void				processRow(int row, const std::vector<float>&);
	bool				allRowsTouched() const { return mTouchedRows == getHeight(); }

private:
	std::atomic<int>	mTouchedRows{ 0 };
	std::mutex			mWriteLock;
	int					mFftVectorStartIndex;
};

typedef std::unique_ptr<StftSurface> StftSurfaceRef;

} // !namespace cistft

#endif // !CIEQ_INCLUDE_STFT_SURFACE_H_