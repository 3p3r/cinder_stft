#include "palette_manager.h"

#include <cinder/audio/Utilities.h>
#include <cinder/params/Params.h>

namespace cieq {
namespace palette {

Manager::Manager()
	: mActivePalette(0)
	, mLinearCoefficient(1024)
	, mDbDivisor(100)
	, mConvertToDb(false)
	, mMinThreshold(0.0f)
	, mMaxThreshold(1.0f)
{}

Manager& Manager::instance()
{
	static Manager _inst;
	return _inst;
}

void Manager::setActivePalette(int palette_index)
{
	if (palette_index < 0) return;
	mActivePalette = palette_index % 6;
}

const ci::Color& Manager::getActivePaletteColor(float FFT_value)
{
	int _active_palette = mActivePalette; // copy in case it changed during execution
	float min = mMinThreshold;
	float max = mMaxThreshold;

	const float value = mConvertToDb ? ci::audio::linearToDecibel(FFT_value) / mDbDivisor : mLinearCoefficient * FFT_value;

	if (_active_palette == 0)
		return getColor<MatlabJet>(value, min, max);
	else if (_active_palette == 1)
		return getColor<MatlabHot>(value, min, max);
	else if (_active_palette == 2)
		return getColor<MPLSummer>(value, min, max);
	else if (_active_palette == 3)
		return getColor<MPLPaired>(value, min, max);
	else if (_active_palette == 4)
		return getColor<MPLOcean>(value, min, max);
	else if (_active_palette == 5)
		return getColor<MPLWinter>(value, min, max);
	else
		return getColor<MatlabJet>(value, min, max);
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

const ci::Color& Manager::getActivePaletteMinColor()
{
	int _active_palette = mActivePalette; // copy in case it changed during execution

	if (_active_palette == 0)
		return MatlabJet::palette[0];
	else if (_active_palette == 1)
		return MatlabHot::palette[0];
	else if (_active_palette == 2)
		return MPLSummer::palette[0];
	else if (_active_palette == 3)
		return MPLPaired::palette[0];
	else if (_active_palette == 4)
		return MPLOcean::palette[0];
	else if (_active_palette == 5)
		return MPLWinter::palette[0];
	else
		return ci::Color::black();
}

void Manager::addToGui(cinder::params::InterfaceGl* const gui)
{
	gui->addText("Palette settings. Type:");
	gui->addText("0 --> Matlab JET");
	gui->addText("1 --> Matlab HOT");
	gui->addText("2 --> MPL Summer");
	gui->addText("3 --> MPL Paired");
	gui->addText("4 --> MPL Ocean");
	gui->addText("5 --> MPL Winter");

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
	// -----------------------------------------------
}

void Manager::removeFromGui(cinder::params::InterfaceGl* const gui)
{
	/* no op */
}

}} //!cieq::palette