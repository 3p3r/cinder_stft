#ifndef CIEQ_INCLUDE_SMART_SURFACE_H_
#define CIEQ_INCLUDE_SMART_SURFACE_H_

#include <atomic>
#include <mutex>

#include <cinder/Surface.h>

namespace cieq {

template<typename T>
class SmartSurface : public ci::SurfaceT<T>
{
public:
	SmartSurface(int width, int height) : ci::SurfaceT<T>(width, height, false) {}
	SmartSurface() = delete;

	void				fillRow(int row, const std::vector<T>&);
	virtual void		processRow(int row, const std::vector<T>&) = 0;
	bool				allColsTouched() const { return mTouchedCols == getWidth(); }

private:
	std::atomic<int>	mTouchedCols{ 0 };
	std::mutex			mWriteLock;
};

template<typename T>
void SmartSurface<T>::fillRow(int row, const std::vector<T>& data)
{
	CI_ASSERT_MSG(row < getHeight(), "SmartSurface rows exceeded height.");
	std::lock_guard<std::mutex> _lock(mWriteLock);
	processRow(row, data);
	mTouchedCols++;
}

typedef SmartSurface<float> SmartSurface32f;

class SpectralSurface final : public SmartSurface32f
{
public:
	SpectralSurface(int width, int height);
	void processRow(int row, const std::vector<float>&) override final;
};

typedef std::unique_ptr<SpectralSurface> SpectralSurfaceRef;

} // !namespace cieq

#endif // !CIEQ_INCLUDE_SMART_SURFACE_H_