#include "stft_surface.h"

#include <cinder/audio/Utilities.h>

namespace cieq {

namespace {
//! A Matlab "Jet" palleted color composition implementation, adapted to Cinder.
//! \note original work: http://paulbourke.net/texture_colour/colourspace/
//! \note taken from: http://stackoverflow.com/a/7811134/1055628
static inline ci::Color GetColour(float v, float vmin, float vmax)
{
	ci::Color c{ ci::Color::white() }; // white
	float dv;

   if (v < vmin)
      v = vmin;
   if (v > vmax)
      v = vmax;
   dv = vmax - vmin;

   if (v < (vmin + 0.25f * dv)) {
      c.r = 0.0f;
      c.g = 4.0f * (v - vmin) / dv;
   } else if (v < (vmin + 0.5f * dv)) {
      c.r = 0.0f;
      c.b = 1.0f + 4.0f * (vmin + 0.25f * dv - v) / dv;
   } else if (v < (vmin + 0.75f * dv)) {
      c.r = 4.0f * (v - vmin - 0.5f * dv) / dv;
      c.b = 0.0f;
   } else {
      c.g = 1.0f + 4.0f * (vmin + 0.75f * dv - v) / dv;
      c.b = 0.0f;
   }

   return c;
}

}

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
		auto c = GetColour(m, 0.0f, 1.0f);
		surface_iter.r() = c.r;
		surface_iter.g() = c.g;
		surface_iter.b() = c.b;
	}
}

} //!cieq