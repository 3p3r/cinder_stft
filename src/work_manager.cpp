#include "work_manager.h"
#include "work_client.h"
#include "work_request.h"

namespace cieq {
namespace work {

Manager::Manager(std::size_t num_threads /*= 4*/)
	: mWorker(mIoService)
{
	for (auto count = num_threads; count > 0; --count)
	{
		mThreadPool.create_thread(boost::bind(&boost::asio::io_service::run, &mIoService));
	}
}

Manager::~Manager()
{
	mIoService.stop();
	mThreadPool.join_all();
}

void Manager::run(std::shared_ptr< Client > requester, std::unique_ptr< Request > request)
{
	if (requester && request)
	{
		request->run(); //run request
		requester->handle(std::move(request)); //call client for recycling
	}
}

void Manager::post(const std::shared_ptr< Client >& requester, std::unique_ptr< Request >& request)
{
	if (requester && request) //HACK ALERT: this is due to lack of official support of moving unique_ptr's inside a lambda!
		mIoService.post(std::bind([=](std::unique_ptr< Request >& w){ run(requester, std::move(w)); }, std::move(request)));
}

}} //!cieq::work