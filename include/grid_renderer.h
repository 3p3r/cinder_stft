#ifndef CISTFT_INCLUDE_GRID_RENDERER_H_
#define CISTFT_INCLUDE_GRID_RENDERER_H_

#include <cinder/Color.h>

namespace cinder {
namespace params {
class InterfaceGl;
}} //!ci::params

namespace cistft {

class AppGlobals;

class GridRenderer
{
public:
	GridRenderer(AppGlobals&);
	
	void			draw();

	void			setHorizontalBoundary(float min = 0.0f, float max = 1.0f);
	void			setVerticalBoundary(float min = 0.0f, float max = 1.0f);

	void			setupPreLaunchGUI(cinder::params::InterfaceGl* const);
	void			setupPostLaunchGUI(cinder::params::InterfaceGl* const);

	void			setHorizontalUnit(const std::string& str);
	void			setVerticalUnit(const std::string& str);

private:
	AppGlobals&		mGlobals;

private:
	int				mStepX;
	int				mStepY;
	int				mLabelFrequency; // per step
	float			mLabelMargin;
	float			mMaxX;
	float			mMaxY;
	float			mMinX;
	float			mMinY;
	ci::Color		mLabelColor;
	ci::Color		mGridColor;
	bool			mVisible;
	std::string		mVerticalUnit;
	std::string		mVerticalUnitTextToDraw;
	std::string		mHorizontalUnit;
	std::string		mHorizontalUnitTextToDraw;
};

} // !namespace cistft

#endif // !CISTFT_INCLUDE_GRID_RENDERER_H_