#include "smart_surface.h"

#include <cinder/audio/Utilities.h>

namespace cieq {

SpectralSurface::SpectralSurface(int width, int height)
	: SmartSurface32f(width, height)
{}

void SpectralSurface::processRow(int row, const std::vector<float>& spectrum)
{
	auto surface_iter = getIter();
	while (surface_iter.mY != row) {
		surface_iter.line(); //seek row
	}

	//! color logic goes here
	while (surface_iter.pixel())
	{
		auto m = ci::audio::linearToDecibel(spectrum[surface_iter.mX]) / 100;
		surface_iter.r() = m;
		surface_iter.g() = m;
		surface_iter.b() = 1.0f - m;
	}
}

} //!cieq