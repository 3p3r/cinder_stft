#ifndef CISTFT_INCLUDE_PALETTE_MANAGER_H_
#define CISTFT_INCLUDE_PALETTE_MANAGER_H_

#include "color_pallete.h"

#include <atomic>

namespace cinder {
namespace params {
class InterfaceGl;
}} //!ci::params

namespace cistft {
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

	void				addToGui(cinder::params::InterfaceGl* const);
	void				removeFromGui(cinder::params::InterfaceGl* const);

private:
	Manager();
	std::atomic<int>	mActivePalette;
	std::atomic<float>	mLinearCoefficient;
	std::atomic<float>	mDbDivisor;
	std::atomic<float>	mMinThreshold;
	std::atomic<float>	mMaxThreshold;
	std::atomic<bool>	mConvertToDb;
};

}} // !namespace cistft::palette

#endif // !CISTFT_INCLUDE_PALETTE_MANAGER_H_