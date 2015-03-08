#ifndef CIEQ_INCLUDE_COLOR_PALETTE_H_
#define CIEQ_INCLUDE_COLOR_PALETTE_H_

#include <array>
#include <cinder/Color.h>

namespace cieq {
namespace palette {

struct MatlabJet		{ static const std::array<const ci::Color, 64> palette; };
struct MatlabHot		{ static const std::array<const ci::Color, 64> palette; };
struct MPLCopper		{ static const std::array<const ci::Color, 128> palette; };
struct MPLPaired		{ static const std::array<const ci::Color, 128> palette; };
struct MPLOcean			{ static const std::array<const ci::Color, 128> palette; };
struct GMTJet			{ static const std::array<const ci::Color, 256> palette; };

template<typename T>
inline static const ci::Color& getColor(float value, float vmin, float vmax)
{
	float vd = vmax - vmin;

	// saturate if out of bounds
	if (value < vmin) value = vmin;
	if (value > vmax) value = vmax;

	return T::palette[static_cast<int>(((value - vmin) / vd) * (std::tuple_size<decltype(T::palette)>::value - 1))];
}

}} // !namespace cieq::palette

#endif // !CIEQ_INCLUDE_COLOR_PALETTE_H_