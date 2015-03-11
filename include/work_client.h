#ifndef CIEQ_INCLUDE_WORK_CLIENT_H_
#define CIEQ_INCLUDE_WORK_CLIENT_H_

#include "work_manager.h"
#include "work_request.h"

namespace cistft {
namespace work {

/*!
 * \class Client
 * \namespace cistft::work
 *
 * \brief the external interface of Work Manager. can be used
 * to submit works to the work manager.
 *
 * \note SHOULD be only constructed to be shared pointers.
 * \note Work Manager keeps these guys
 * by shared_ptr's and thus, have a shared ownership over them in conjunction with
 * whomever constructs them.
 */

class Client : public std::enable_shared_from_this< Client >
{
public:
	//! \brief Client constructor, should NOT be called directly. use factory method "make"
	Client(Manager& manager) : mManager(manager) {}

	//! \brief callback, called inside a thread as soon as a request is done.
	//! \note this is called inside a thread! it is SHARED with the main thread. be aware!
	virtual void		handle(RequestRef) { /*no op*/ }
	//! \brief requests a new work from the manager.
	virtual void		request(RequestRef& work) { mManager.post(shared_from_this(), work); }

private:
	Manager&			mManager;
};

/* IMPLEMENTATION */
template<class T, class... Args>
static ClientRef make_client(Manager& manager, Args... args)
{
	// check if T is a subclass of Client.
	static_assert(std::is_base_of<Client, T>::value,
		"ClientRef factory method only accepts types derived from cistft::work::Client");
	// check if T properly overloaded the constructor
	static_assert(std::is_constructible<T, Manager&>::value,
		"ClientRef does not provide a constructor for cistft::work::Client");
	// copy elision will happen here by the compiler, no std::forward required.
	return std::shared_ptr<T>(new T(manager, args...));
}

/* C++11's thread_local define */
#ifndef thread_local
#define thread_local __declspec(thread)
#endif // !thread_local

}}

#endif //!CIEQ_INCLUDE_WORK_CLIENT_H_