#ifndef CISTFT_INCLUDE_PALETTE_MANAGER_H_
#define CISTFT_INCLUDE_PALETTE_MANAGER_H_

#include "color_pallete.h"

#include <functional>
#include <atomic>
#include <mutex>

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

	void				setupPreLaunchGUI(cinder::params::InterfaceGl* const);
	void				setupPostLaunchGUI(cinder::params::InterfaceGl* const);

private:
	Manager();
	std::atomic<int>	mActivePalette;
	std::atomic<float>	mLinearCoefficient;
	std::atomic<float>	mDbDivisor;
	std::atomic<float>	mMinThreshold;
	std::atomic<float>	mMaxThreshold;
	std::atomic<bool>	mConvertToDb;

private:
	std::mutex			mColorProviderLock;
	std::function < const ci::Color&(float, float, float) >
						mColorProvider;
};

}} // !namespace cistft::palette

#endif // !CISTFT_INCLUDE_PALETTE_MANAGER_H_