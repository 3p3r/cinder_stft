#ifndef CIEQ_INCLUDE_PALETTE_MANAGER_H_
#define CIEQ_INCLUDE_PALETTE_MANAGER_H_

#include "color_pallete.h"

#include <atomic>

namespace cieq {
namespace palette {

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
	const ci::Color&	getActivePaletteMinColor();

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

#endif // !CIEQ_INCLUDE_PALETTE_MANAGER_H_