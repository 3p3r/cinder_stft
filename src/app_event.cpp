#include "app_event.h"

namespace cieq
{

void AppEvent::triggerMouse(const std::function< void(float, float) >& fn)
{
	mMouseEventsStack.connect(fn);
}

void AppEvent::triggerKeyboard(const std::function< void(int) >& fn)
{
	mKeyboardEventsStack.connect(fn);
}

void AppEvent::processMouseEvents(float x, float y)
{
	if (mMouseEventsStack.empty()) return;

	mMouseEventsStack(x, y);

	mMouseEventsStack.disconnect_all_slots();
}

void AppEvent::processKeybaordEvents(char key)
{
	if (mKeyboardEventsStack.empty()) return;

	mKeyboardEventsStack(key);

	mKeyboardEventsStack.disconnect_all_slots();
}

} // !namespace cieq