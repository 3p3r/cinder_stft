#include "stft_surface.h"
#include "color_pallete.h"

namespace cieq {

StftSurface::StftSurface(int width, int height, int fft_vector_index)
	: ci::Surface32f(width, height, false)
	, mFftVectorStartIndex(fft_vector_index)
{}

void StftSurface::fillRow(int row, const std::vector<float>& data)
{
	std::lock_guard<std::mutex> _lock(mWriteLock);
	processRow(row, data);
	mTouchedRows++;
}

void StftSurface::processRow(int row, const std::vector<float>& spectrum)
{
	auto surface_iter = getIter();
	while (surface_iter.mY != row) {
		surface_iter.line(); //seek row
	}

	//! color logic goes here
	while (surface_iter.pixel())
	{
		auto c = palette::Manager::instance().getActivePaletteColor(spectrum[surface_iter.mX + mFftVectorStartIndex]);
		surface_iter.r() = c.r;
		surface_iter.g() = c.g;
		surface_iter.b() = c.b;
	}
}

} //!cieq