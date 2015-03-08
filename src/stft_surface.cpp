#include "stft_surface.h"
#include "color_pallete.h"

#include <array>

#include <cinder/audio/Utilities.h>

namespace cieq {

StftSurface::StftSurface(int width, int height)
	: ci::Surface32f(width, height, false)
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
		auto m = 1024.0f * spectrum[surface_iter.mX];
		auto c = palette::getColor<palette::MatlabJet>(m, 0.0f, 1.0f);
		surface_iter.r() = c.r;
		surface_iter.g() = c.g;
		surface_iter.b() = c.b;
	}
}

} //!cieq