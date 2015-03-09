#ifndef CIEQ_INCLUDE_STFT_FILTER_H_
#define CIEQ_INCLUDE_STFT_FILTER_H_

namespace cinder {
namespace params {
class InterfaceGl;
}} //!ci::params

namespace cieq {

class AppGlobals;

class StftFilter
{
public:
	StftFilter(AppGlobals&);

	void		addToGui(cinder::params::InterfaceGl* const);
	void		removeFromGui(cinder::params::InterfaceGl* const);

	StftFilter& viewableBins(int val);
	StftFilter& lowPassFrequency(float val);
	StftFilter& highPassFrequency(float val);

	int			getViewableBins() const;
	float		getLowPassFrequency() const;
	float		getHighPassFrequency() const;
	int			getCalculatedFftSize() const;
	float		getActualLowPassFrequency() const;
	float		getActualHighPassFrequency() const;
	int			getMagnitudeIndexStart() const;
	int			getMagnitudeIndexEnd() const;

private:
	AppGlobals&	mGlobals;
	int			mViewableBins;
	float		mLowPassFrequency;
	float		mHighPassFrequency;
	int			mCalculatedFftSize;
	float		mActualLowPassFrequency;
	float		mActualHighPassFrequency;
	int			mMagnitudeIndexStart;
	int			mMagnitudeIndexEnd;

private:
	void		calculate();
};

}

#endif //!CIEQ_INCLUDE_STFT_FILTER_H_