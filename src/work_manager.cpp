#include "work_manager.h"

namespace cieq {
namespace work {

void Manager::add(std::shared_ptr<Client> requester, std::unique_ptr<Request> request)
{
	//first take ownership appropriately locally inside our memory.
	mClientWorks.push_back(std::make_pair(requester, std::move(request)));
	//Then pass to pending queue to process
	mPending.push(mClientWorks.size() - 1);
}

void Manager::update()
{
	//while pending pop --> post to ASIO
	//while finished pop and bounded to max --> call clients by finished work
}

}} //!cieq::work