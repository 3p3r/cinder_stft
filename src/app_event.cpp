#include "app_event.h"

namespace cieq
{

void AppEvent::addMouseCallback(const std::function< void(float, float) >& fn)
{
	mMouseEventsStack.connect(fn);
}

void AppEvent::addKeyboardCallback(const std::function< void(int) >& fn)
{
	mKeyboardEventsStack.connect(fn);
}

void AppEvent::fireMouseCallbacks(float x, float y)
{
	if (mMouseEventsStack.empty()) return;

	mMouseEventsStack(x, y);
}

void AppEvent::fireKeyboardCallbacks(char key)
{
	if (mKeyboardEventsStack.empty()) return;

	mKeyboardEventsStack(key);
}

} // !namespace cieq