#ifndef CIEQ_INCLUDE_WORK_CLIENT_H_
#define CIEQ_INCLUDE_WORK_CLIENT_H_

#include "work_manager.h"
#include "work_request.h"

namespace cieq {
namespace work {

//! \note a shallow type for Client shared_ptr's
typedef std::shared_ptr< class Client > ClientRef;

/*!
 * \class Client
 * \namespace cieq::work
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
	virtual void		handle(std::unique_ptr< Request >) { /*no op*/ }
	//! \brief requests a new work from the manager.
	virtual void		request(std::unique_ptr< Request >& work) { mManager.post(shared_from_this(), work); }

	template<class T, class... Args>
	static ClientRef	make(Manager& manager, Args... args);

private:
	Manager&			mManager;
};

/* IMPLEMENTATION */
template<class T, class... Args>
ClientRef cieq::work::Client::make(Manager& manager, Args... args)
{
	// check if T is a subclass of Client.
	static_assert(std::is_base_of<Client, T>::value,
		"ClientRef factory method only accepts types derived from cieq::work::Client");
	// check if T properly overloaded the constructor
	static_assert(std::is_constructible<T, Manager&>::value,
		"ClientRef does not provide a constructor for cieq::work::Client")
	// copy elision will happen here by the compiler, no std::forward required.
	return std::shared_ptr<T>(new T(manager, args...));
}

}}

#endif //!CIEQ_INCLUDE_WORK_CLIENT_H_