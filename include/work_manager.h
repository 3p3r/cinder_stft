#ifndef CIEQ_INCLUDE_WORK_MANAGER_H_
#define CIEQ_INCLUDE_WORK_MANAGER_H_

#include <boost/asio/io_service.hpp>
#include <boost/thread/thread.hpp>

namespace cieq {
namespace work {

/*!
 * \class Manager
 * \namespace cieq::work
 *
 * \brief A work manager implemented with a dependency on Boost.Asio
 * handles work requests from the main thread and executes them in the
 * background.
 */
class Manager
{
public:
	//! \brief constructor of a work manager, accepts number of worker threads.
	Manager(std::size_t num_threads = 4);
	//! \brief joins all threads and stops IO service.
	virtual ~Manager();

	//! \brief runs a work request synchronously.
	void							run(std::shared_ptr< class Client >, std::unique_ptr< class Request >);
	//! \brief send a work request to the worker pool and runs it asynchronously.
	void							post(const std::shared_ptr< class Client >&, std::unique_ptr< class Request >&);

private:
	boost::asio::io_service			mIoService;
	boost::thread_group				mThreadPool;
	boost::asio::io_service::work	mWorker;
};

}}

#endif //!CIEQ_INCLUDE_WORK_MANAGER_H_