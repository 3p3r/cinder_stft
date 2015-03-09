#ifndef CIEQ_INCLUDE_STFT_FILTER_H_
#define CIEQ_INCLUDE_STFT_FILTER_H_

namespace cieq {

class AppGlobals;

class StftFilter
{
public:
	StftFilter(AppGlobals&);

	void		calculate();

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
};

}

#endif //!CIEQ_INCLUDE_STFT_FILTER_H_