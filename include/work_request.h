#ifndef CIEQ_INCLUDE_WORK_REQUEST_H_
#define CIEQ_INCLUDE_WORK_REQUEST_H_

#include "work_manager.h"

#include <memory>

namespace cieq {
namespace work {

/*!
 * \class Request
 * \namespace cieq::work
 *
 * \brief One unit of work that will be passed to Work Manager.
 * it is supposed to run an expensive operation in background.
 *
 * \note ONLY a unique_ptr of this guy SHOULD be constructed. no shared
 * ownership what-so-ever!
 * \note subclass Request to get custom processing done.
 * \note use Request::make as the factory function.
 */

class Request
{
public:
	//! \brief main processing call. Background thread will call this.
	virtual void		run() {};
};

//! \brief factory function for work requests. returns a unique_ptr.
template<class T, class... Args>
static RequestRef make_request(Args... args)
{
	// check if T is a subclass of Request.
	static_assert(std::is_base_of<Request, T>::value,
		"RequestRef factory method only accepts types derived from cieq::work::Request");
	// copy elision will happen here by the compiler, no std::move required.
	return std::unique_ptr<T>(new T(args...));
}

}}

#endif //!CIEQ_INCLUDE_WORK_REQUEST_H_