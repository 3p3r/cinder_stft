#ifndef CISTFT_INCLUDE_STFT_FILTER_H_
#define CISTFT_INCLUDE_STFT_FILTER_H_

namespace cinder {
namespace params {
class InterfaceGl;
}} //!ci::params

namespace cistft {

class AppGlobals;

class StftFilter
{
public:
	StftFilter(AppGlobals&);

	void		addToGui(cinder::params::InterfaceGl* const);
	void		removeFromGui(cinder::params::InterfaceGl* const);

	StftFilter& minimumViewableBins(int val);
	StftFilter& lowPassFrequency(float val);
	StftFilter& highPassFrequency(float val);

	int			getMinimumViewableBins() const;
	float		getLowPassFrequency() const;
	float		getHighPassFrequency() const;
	
	int			getActualViewableBins() const;
	float		getActualLowPassFrequency() const;
	float		getActualHighPassFrequency() const;
	
	int			getCalculatedFftSize() const;
	int			getMagnitudeIndexStart() const;

private:
	AppGlobals&	mGlobals;
	
	int			mMinimumViewableBins;
	float		mLowPassFrequency;
	float		mHighPassFrequency;

	int			mActualViewableBins;
	float		mActualLowPassFrequency;
	float		mActualHighPassFrequency;

	int			mCalculatedFftSize;
	int			mMagnitudeIndexStart;

private:
	void		calculate();
};

}

#endif //!CISTFT_INCLUDE_STFT_FILTER_H_