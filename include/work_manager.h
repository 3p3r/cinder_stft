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
	Manager(std::size_t num_threads = 4);
	virtual ~Manager();

	void							run(std::shared_ptr< class Client >, std::unique_ptr< class Request >);
	void							post(std::shared_ptr< class Client >, std::unique_ptr< class Request >);

private:
	boost::asio::io_service			mIoService;
	boost::thread_group				mThreadPool;
	boost::asio::io_service::work	mWorker;
};

}}

#endif //!CIEQ_INCLUDE_WORK_MANAGER_H_