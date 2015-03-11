#ifndef CIEQ_INCLUDE_COLOR_PALETTE_H_
#define CIEQ_INCLUDE_COLOR_PALETTE_H_

#include <array>

#include <cinder/Color.h>

namespace cistft {
namespace palette {

struct MatlabJet		{ static const std::array<const ci::Color, 64> palette; };
struct MatlabHot		{ static const std::array<const ci::Color, 64> palette; };
struct MPLSummer		{ static const std::array<const ci::Color, 128> palette; };
struct MPLPaired		{ static const std::array<const ci::Color, 128> palette; };
struct MPLOcean			{ static const std::array<const ci::Color, 128> palette; };
struct MPLWinter		{ static const std::array<const ci::Color, 128> palette; };

template<typename T>
inline static const ci::Color& getColor(float value, float vmin, float vmax)
{
	float vd = vmax - vmin;

	// saturate if out of bounds
	if (value < vmin) value = vmin;
	if (value > vmax) value = vmax;

	return T::palette[static_cast<int>(((value - vmin) / vd) * (std::tuple_size<decltype(T::palette)>::value - 1))];
}

}} // !namespace cistft::palette

#endif // !CIEQ_INCLUDE_COLOR_PALETTE_H_