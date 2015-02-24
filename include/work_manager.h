#ifndef CIEQ_INCLUDE_WORK_MANAGER_H_
#define CIEQ_INCLUDE_WORK_MANAGER_H_

#include <boost/lockfree/queue.hpp>

#include <vector>
#include <memory>

#include "work_request.h"
#include "work_client.h"

namespace cieq {
namespace work {

/*!
 * \class Manager
 * \namespace cieq::work
 *
 * \brief A work manager implemented with a dependency on Boost.Asio
 * handles work requests from the main thread and executes them in the
 * background. Has one lock-free queue as pending work requests and one
 * lock-free queue for finished works.
 *
 * \note work requests will be passed around as unique_ptr's.
 * \note use work clients to post work to the work manager.
 * \note subclass cieq::work::Work to have custom works done.
 * \note when a Client posts a work, Manager pushes a std::pair of Client/Work
 * to the pending work queue. On each update() call, Manager pops pending works
 * and posts them to the underlying thread-pool. when job's done, the same
 * std::pair of Client/Work will be pushed to finished works queue. In the end,
 * each std::pair of Clients in finished works queue will be called with the
 * unique_ptr of work which is finished (if the work needs to be recycled.)
 * \note there's no limit on how many works can be posted (in theory) but there's
 * a limit to number of finished work calls per update. This is simply because
 * there might be a case that constantly works are being requested and added
 * and also they are finished before update() exits, in this situation if there's
 * no maximum finished works per update(), program will fall into an endless loop.
 */
class Manager
{
	// Clients can only add works to me
	friend class Client;

	// Used for finished work storage. Integers are finished work id's.
	using queue		= boost::lockfree::queue < int > ;
	// Used to take ownership of Clients + Works.
	using memory	= std::vector < std::pair < std::shared_ptr < Client >, std::unique_ptr< Request > > > ;

public:
	// \note safe to call from producer (single) thread only.
	void			update();

protected:
	// \note safe to call from producer (single) thread only.
	void			add(std::shared_ptr<Client>, std::unique_ptr<Request>);

private:
	queue			mPending;
	queue			mFinished;
	memory			mClientWorks;
};

}}

#endif //!CIEQ_INCLUDE_WORK_MANAGER_H_