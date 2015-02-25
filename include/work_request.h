#ifndef CIEQ_INCLUDE_WORK_REQUEST_H_
#define CIEQ_INCLUDE_WORK_REQUEST_H_

#include <memory>
#include "cinder\app\App.h"

namespace cieq {
namespace work {

/*!
 * \class Request
 * \namespace cieq::work
 *
 * \brief One unit of work that will be passed to Work Manager.
 * it is supposed to run an expensive operation in background.
 *
 * \note ONLY a unique_ptr of this guy can be constructed. no shared
 * ownership what-so-ever!
 * \note notifies Manager when the work is finished.
 * \note subclass Request to get custom processing done.
 */

using RequestRef = std::unique_ptr< class Request >;
class Request
{
public:
	virtual void		run() {};
	
	template<class T, class... Args, typename std::enable_if<std::is_base_of<Request, T>::value>::type* = nullptr>
	static RequestRef	make(Args...);
};

template<class T, class... Args, typename std::enable_if<std::is_base_of<Request, T>::value>::type*>
RequestRef cieq::work::Request::make(Args... args)
{
	return std::unique_ptr<T>(new T(args...));
}

}}

#endif //!CIEQ_INCLUDE_WORK_REQUEST_H_