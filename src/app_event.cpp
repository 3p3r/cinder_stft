#include "app_event.h"

namespace cieq
{

void AppEvent::addMouseEvent(const std::function< void(float, float) >& fn)
{
	mMouseEventsStack.connect(fn);
}

void AppEvent::addKeyboardEvent(const std::function< void(int) >& fn)
{
	mKeyboardEventsStack.connect(fn);
}

void AppEvent::processMouseEvents(float x, float y)
{
	if (mMouseEventsStack.empty()) return;

	mMouseEventsStack(x, y);
}

void AppEvent::processKeybaordEvents(char key)
{
	if (mKeyboardEventsStack.empty()) return;

	mKeyboardEventsStack(key);
}

} // !namespace cieq