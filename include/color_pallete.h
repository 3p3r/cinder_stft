#ifndef CIEQ_INCLUDE_COLOR_PALETTE_H_
#define CIEQ_INCLUDE_COLOR_PALETTE_H_

#include <array>
#include <atomic>

#include <cinder/Color.h>

namespace cieq {
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

class Manager
{
public:
	static Manager&		instance();

	void				setActivePalette(int palette_index);
	int					getActivePalette() const { return mActivePalette; }

	void				setLinearCoefficient(float coeff);
	float				getLinearCoefficient() const { return mLinearCoefficient; }

	void				setDbDivisor(float div);
	float				getDbDivisor() const { return mDbDivisor; }

	void				setMinThreshold(float val);
	float				getMinThreshold() const { return mMinThreshold; }

	void				setMaxThreshold(float val);
	float				getMaxThreshold() const { return mMaxThreshold; }

	void				setConvertToDb(bool convert);
	bool				getConvertToDb() const { return mConvertToDb; }

	const ci::Color&	getActivePaletteColor(float FFT_value);

private:
	Manager();
	std::atomic<int>	mActivePalette;
	std::atomic<float>	mLinearCoefficient;
	std::atomic<float>	mDbDivisor;
	std::atomic<float>	mMinThreshold;
	std::atomic<float>	mMaxThreshold;
	std::atomic<bool>	mConvertToDb;
};

}} // !namespace cieq::palette

#endif // !CIEQ_INCLUDE_COLOR_PALETTE_H_