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
	/*!
	 * \brief adds a mouse event processor to events stack.
	 * \note no matter how many events you may want to occur,
	 * the all get fired in app::MouseDown method.
	 */ 
	void addMouseEvent(const std::function< void(float, float) >&);

	/*!
	 * \brief adds a keyboard event processor to events stack.
	 * \note no matter how many events you may want to occur,
	 * the all get fired in app::KeyDown method.
	 */ 
	void addKeyboardEvent(const std::function< void(int) >&);

public:
	/*!
	 * \brief fires all events in the stack for an incoming mouse
	 * event. X, and Y coordinates will be passed inside.
	 */
	void processMouseEvents(float x, float y);

	/*!
	 * \brief fires all events in the stack for an incoming keyboard
	 * event. character code will be passed inside.
	 */
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