#ifndef CIEQ_INCLUDE_WORK_REQUEST_H_
#define CIEQ_INCLUDE_WORK_REQUEST_H_

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
class Request
{

};

}}

#endif //!CIEQ_INCLUDE_WORK_REQUEST_H_