#ifndef CIEQ_INCLUDE_GRID_RENDERER_H_
#define CIEQ_INCLUDE_GRID_RENDERER_H_

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

	struct
	{
		// to be configured via GUI

		int			mStepX;
		int			mStepY;
		int			mLabelFrequency; // per step
		float		mLabelMargin;
		float		mMaxX;
		float		mMaxY;
		float		mMinX;
		float		mMinY;
		ci::Color	mLabelColor;
		ci::Color	mGridColor;
		bool		mVisible;

	} mConfiguration;

	void			resetConfiguration();

	void			addToGui(cinder::params::InterfaceGl* const);
	void			removeFromGui(cinder::params::InterfaceGl* const);

private:
	AppGlobals&		mGlobals;
};

} // !namespace cistft

#endif // !CIEQ_INCLUDE_GRID_RENDERER_H_