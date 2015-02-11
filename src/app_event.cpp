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

void AppEvent::addMouseDragEvent(const std::function< void(float, float, bool, bool, bool) >& fn)
{
	mMouseDragEventsStack.connect(fn);
}

void AppEvent::processMouseDragEvents(float x, float y, bool l, bool m, bool r)
{
	if (mMouseDragEventsStack.empty()) return;

	mMouseDragEventsStack(x, y, l, m, r);
}

} // !namespace cieq