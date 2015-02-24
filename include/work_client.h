#ifndef CIEQ_INCLUDE_WORK_CLIENT_H_
#define CIEQ_INCLUDE_WORK_CLIENT_H_

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
class Client
{

};

}}

#endif //!CIEQ_INCLUDE_WORK_CLIENT_H_