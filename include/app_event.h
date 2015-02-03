#ifndef CIEQ_INCLUDE_APP_EVENT_H_
#define CIEQ_INCLUDE_APP_EVENT_H_

#include <boost/signals2/signal.hpp>

namespace cieq
{

// I make an alias of boost here because
// writing boost::signal2 is unnecessarilly
// verbose.
namespace signal = boost::signals2;

/*!
 * \class AppEvent
 * \brief The class that handles mouse and keyboard input,
 * and potentially other physical inputs (not audio tho!)
 */
class AppEvent
{
public:
	void addMouseEvent(const std::function< void(float, float) >&);
	void addKeyboardEvent(const std::function< void(int) >&);

public:
	void processMouseEvents(float x, float y);
	void processKeybaordEvents(char key);

private:
	using mouseEvent	= signal::signal < void(float, float) > ;
	using keyboardEvent = signal::signal < void(char) > ;

private:
	mouseEvent		mMouseEventsStack;
	keyboardEvent	mKeyboardEventsStack;
};

} //!cieq

#endif //!CIEQ_INCLUDE_APP_EVENT_H_