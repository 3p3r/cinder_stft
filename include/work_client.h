#ifndef CIEQ_INCLUDE_WORK_CLIENT_H_
#define CIEQ_INCLUDE_WORK_CLIENT_H_

#include "work_manager.h"

namespace cieq {
namespace work {

/*!
 * \class Client
 * \namespace cieq::work
 *
 * \brief the external interface of Work Manager. can be used
 * to submit works to the work manager.
 *
 * \note can be ONLY constructed to be shared pointers.
 * \note Work Manager keeps these guys
 * by shared_ptr's and thus, have a shared ownership over them in conjunction with
 * whomever constructs them.
 * \note Work Manager assigns ID to these guys.
 */

using ClientRef = std::shared_ptr< class Client >;
class Client : public std::enable_shared_from_this< Client >
{
public:
	Client(Manager& manager) : mManager(manager) {}

	virtual void		handle(std::unique_ptr< class Request >) {}
	virtual void		request(std::unique_ptr<class Request > work) { mManager.post(shared_from_this(), std::move(work)); }

	template<class T, class... Args, typename std::enable_if<std::is_base_of<Client, T>::value>::type* = nullptr>
	static ClientRef	make(Manager& manager, Args... args);

private:
	Manager&			mManager;
};

template<class T, class... Args, typename std::enable_if<std::is_base_of<Client, T>::value>::type*>
ClientRef cieq::work::Client::make(Manager& manager, Args... args)
{
	return std::shared_ptr<T>(new T(manager, args...));
}

}}

#endif //!CIEQ_INCLUDE_WORK_CLIENT_H_