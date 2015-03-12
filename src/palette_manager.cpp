#include "palette_manager.h"

#include <cinder/audio/Utilities.h>
#include <cinder/params/Params.h>

namespace cistft {
namespace palette {

Manager::Manager()
	: mActivePalette(0)
	, mLinearCoefficient(1024)
	, mDbDivisor(100)
	, mConvertToDb(false)
	, mMinThreshold(0.0f)
	, mMaxThreshold(1.0f)
	, mColorProvider([](float v, float min, float max)->const ci::Color&{ return getColor<MatlabJet>(v, min, max); })
{}

Manager& Manager::instance()
{
	static Manager _inst;
	return _inst;
}

namespace {
static int NUM_PALETTES = 14;
} //!namespace

void Manager::setActivePalette(int palette_index)
{
	if (palette_index < 0 || mActivePalette == palette_index) return;

	mActivePalette = palette_index % NUM_PALETTES;

	std::lock_guard<std::mutex> _lock(mColorProviderLock);

	switch (mActivePalette)
	{
	case 0:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MatlabJet>(v, min, max); };
		break;
	case 1:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MatlabHot>(v, min, max); };
		break;
	case 2:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MPLSummer>(v, min, max); };
		break;
	case 3:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MPLPaired>(v, min, max); };
		break;
	case 4:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MPLOcean>(v, min, max); };
		break;
	case 5:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MPLWinter>(v, min, max); };
		break;
	case 6:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<OceanLakeLandSnow>(v, min, max); };
		break;
	case 7:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<SVGBhw322>(v, min, max); };
		break;
	case 8:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MPLGnuplot>(v, min, max); };
		break;
	case 9:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MPLFlag>(v, min, max); };
		break;
	case 10:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<NCVManga>(v, min, max); };
		break;
	case 11:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MPLPrism>(v, min, max); };
		break;
	case 12:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<SVGLindaa07>(v, min, max); };
		break;
	case 13:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<SVGGallet13>(v, min, max); };
		break;
	default:
		mColorProvider = [](float v, float min, float max)->const ci::Color&{ return getColor<MatlabJet>(v, min, max); };
		break;
	}
}

const ci::Color& Manager::getActivePaletteColor(float FFT_value)
{
	float min = mMinThreshold;
	float max = mMaxThreshold;

	const float value = mConvertToDb ? ci::audio::linearToDecibel(FFT_value) / mDbDivisor : mLinearCoefficient * FFT_value;
	
	return mColorProvider(value, min, max);
}

void Manager::setLinearCoefficient(float coeff)
{
	if (coeff < 0.0f) return;
	mLinearCoefficient = coeff;
}

void Manager::setDbDivisor(float div)
{
	if (div == 0.0f) return;
	mDbDivisor = div;
}

void Manager::setConvertToDb(bool convert)
{
	mConvertToDb = convert;
}

void Manager::setMinThreshold(float val)
{
	if (val < 0.0f || val > mMaxThreshold) return;
	mMinThreshold = val;
}

void Manager::setMaxThreshold(float val)
{
	if (val < 0.0f || val < mMinThreshold) return;
	mMaxThreshold = val;
}

namespace {
const static std::string GUI_SEPARATOR("_P");
const static std::string PRE_LAUNCH_TEXT("Palettes will be configurable after you hit START.");
}

void Manager::setupPreLaunchGUI(cinder::params::InterfaceGl* const gui)
{
	gui->addSeparator(GUI_SEPARATOR);
	gui->addText(PRE_LAUNCH_TEXT);
}

void Manager::setupPostLaunchGUI(cinder::params::InterfaceGl* const gui)
{
	gui->removeParam(GUI_SEPARATOR);
	gui->removeParam(PRE_LAUNCH_TEXT);

	gui->addSeparator();
	gui->addText("Palette settings. Type:");
	gui->addText("0 --> Matlab JET");
	gui->addText("1 --> Matlab HOT");
	gui->addText("2 --> MPL Summer");
	gui->addText("3 --> MPL Paired");
	gui->addText("4 --> MPL Ocean");
	gui->addText("5 --> MPL Winter");
	gui->addText("6 --> Ocean Lake Land Snow");
	gui->addText("7 --> SVG bhw3 22");
	gui->addText("8 --> MPL gnuplot");
	gui->addText("9 --> MPL flag");
	gui->addText("10 --> NCV manga");
	gui->addText("11 --> MPL prism");
	gui->addText("12 --> SVG Lindaa07");
	gui->addText("13 --> SVG Gallet13");

	gui->addParam<int>("Color palette",
		[](int p){ palette::Manager::instance().setActivePalette(p); },
		[]()->int{ return palette::Manager::instance().getActivePalette(); });

	gui->addParam<bool>("Convert to dB mode",
		[](bool c){ palette::Manager::instance().setConvertToDb(c); },
		[]()->bool{ return palette::Manager::instance().getConvertToDb(); });

	gui->addParam<float>("dB mode divisor",
		[](float val){ palette::Manager::instance().setDbDivisor(val); },
		[]()->float{ return palette::Manager::instance().getDbDivisor(); });

	gui->addParam<float>("linear mode coefficient",
		[](float val){ palette::Manager::instance().setLinearCoefficient(val); },
		[]()->float{ return palette::Manager::instance().getLinearCoefficient(); });

	gui->addParam<float>("Min color threshold [0, 1]",
		[](float val){ palette::Manager::instance().setMinThreshold(val); },
		[]()->float{ return palette::Manager::instance().getMinThreshold(); });

	gui->addParam<float>("Max color threshold [0, 1]",
		[](float val){ palette::Manager::instance().setMaxThreshold(val); },
		[]()->float{ return palette::Manager::instance().getMaxThreshold(); });
	gui->addSeparator();
}

}} //!cistft::palette