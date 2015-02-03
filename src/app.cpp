#include "app.h"

namespace cieq
{

void InputAnalyzer::prepareSettings(Settings *settings)
{
	settings->enableConsoleWindow(true);
	settings->setResizable(true);
	settings->setTitle("Cinder Audio Equalizer");

	const auto current_display_size = settings->getDisplay()->getSize();
	const auto window_position = (current_display_size * 5) / 100;
	const auto window_size = (current_display_size * 9) / 10;

	settings->setWindowPos(window_position);
	settings->setWindowSize(window_size);
}

void InputAnalyzer::setup()
{

}

void InputAnalyzer::mouseDown(ci::app::MouseEvent event)
{

}

void InputAnalyzer::update()
{

}

void InputAnalyzer::draw()
{

}

} //!namespace cieq

CINDER_APP_NATIVE(cieq::InputAnalyzer, ci::app::RendererGl)