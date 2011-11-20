/*    ___  _________     ____          __         
     / _ )/ __/ ___/____/ __/___ ___ _/_/___ ___ 
    / _  / _// (_ //___/ _/ / _ | _ `/ // _ | -_)
   /____/_/  \___/    /___//_//_|_, /_//_//_|__/ 
                               /___/             

This file is part of the Brute-Force Game Engine, BFG-Engine

For the latest info, see http://www.brute-force-games.com

Copyright (c) 2011 Brute-Force Games GbR

The BFG-Engine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

The BFG-Engine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with the BFG-Engine. If not, see <http://www.gnu.org/licenses/>.
*/

//
// The EventManager coordinates the Threads, EventChannels und the EventExecution
// Incoming means : Communication from the EventManager to the EventLoop
// Outgoing means : Communication from the EventLoop to the EventManager
//
// An EventManager can be used by any class.
// Remember! This will only be used in a threaded environment
//

#include <boost/archive/text_oarchive.hpp>

#include <Base/Cpp.h>
#include <Base/CLogger.h>
#include <Base/NameCurrentThread.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Core/EventManager.h>
#include <EventSystem/Core/network/ClientChannel.h>
#include <EventSystem/Core/network/ServerChannel.h>


EventManager* EventManager::getInstance()
{
	return BFG::Base::CSingletonT<EventManager>::instance();
}

EventManager::EventManager() :
mThreadCount(0),
mExitFlag(false),
mAcceptor(0)
{
	mThread = boost::thread(&EventManager::runThread, this);
}

EventManager::~EventManager(void)
{
	mExitFlag = true;
	// Wait till thread is finished, then do cleanup
	mThread.join();
	// Do real cleanup here
	while ( !mPoolQueue.empty() )
	{
		delete mPoolQueue.front();
		mPoolQueue.pop_front();
	}
}

void EventManager::runThread()
{
	BFG::nameCurrentThread("EventManager");

	size_t operationCounter = 0;
	while(!mExitFlag)
	{
		// Do normal operation
		if (!doOperate())
		{
			++operationCounter;
			if (operationCounter < 10)
				boost::thread::yield();
			else
			{
				boost::thread::sleep
				(
					boost::get_system_time() + boost::posix_time::milliseconds(1)
				);
			}
		}
		else
		{
			operationCounter = 0; 
		}

		if (! mConnections.empty())
			mService.poll();
	}
}

long EventManager::registerEventLoop(EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);
	mEventChannelList[mThreadCount] = ioChannel;
	long wRet = ++mThreadCount;
	mEventChannelList[mThreadCount] = NULL;
	return wRet;
}

void EventManager::unregisterEventLoop(EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);

	int channel_id = getChannelId(ioChannel, false);
	if (channel_id == -1)
		return;

	if (channel_id == (mThreadCount - 1))
	{
		mEventChannelList[channel_id] = NULL;
	}
	else
	{
		mEventChannelList[channel_id] = mEventChannelList[mThreadCount - 1];
		mEventChannelList[mThreadCount - 1] = NULL;
	}	
	--mThreadCount;
}

void EventManager::subscribeEvent(long eventId, EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mExecutionListMutex);

	EventChannelMaskMap::iterator it = mExecutionList.find (eventId);
	long mask = 0;
	if ( it != mExecutionList.end() )
	{	
		mask = (*it).second;
	}	
	int id = getChannelId(ioChannel);
	mExecutionList[eventId] = mask | (1 << id);
}

void EventManager::subscribeEvents(long* , EventChannel* )
{
	assert(false && "TODO: EventManager::SubscribeEvents(long*,EventChannel*)");
}

void EventManager::unsubscribeEvent(long eventId, EventChannel* ioChannel)
{
	boost::mutex::scoped_lock scoped_lock(mExecutionListMutex);

	EventChannelMaskMap::iterator it = mExecutionList.find(eventId);
	long mask = 0;
	if (it != mExecutionList.end())
	{	
		mask = (*it).second;
	}	
	int id = getChannelId(ioChannel);

	if (mask && (1 << id))
	{
		mExecutionList[eventId] = mask - (1 << id);
	}

}

void EventManager::unsubscribeAll(EventChannel* ioChannel)
{
	// Takes a very long time
	boost::mutex::scoped_lock scoped_lock(mExecutionListMutex);

	EventChannelMaskMap::iterator it = mExecutionList.begin();

	int id = getChannelId(ioChannel);
	if (id == -1)
		return;
	
	long channelmask = (1 << id);
	while(it != mExecutionList.end())
	{
		if ((*it).second & channelmask)
		{
			long mask = (*it).second;
			mExecutionList[(*it).first] = (mask - channelmask);
		}
		++it;
	}

}

//!
//! After many thoughts and considerations, this function will be implemented using the Read-Only-Doctrine.
//! All Events have to be considered Read-Only, because there will be many readers.
//!
//! \return true if caller has spend cpu-time

bool EventManager::doOperate()
{
	infolog << "New Tick";
	//! \todo rename InternalMutex to OperationMutex or something
	//! InternalMutex is only for DoOperate function ?
	boost::try_mutex::scoped_try_lock internal_lock(mInternalMutex); 
	if (!internal_lock.owns_lock())
	{
		return false;
	}

	// Aquire all necessary Locks
	boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);

	bool didWork = false;
	for (size_t i = 0; i < mThreadCount; ++i)
	{
		BaseEventPool* pool;
		while(pool = mEventChannelList[i]->getPublishedEventPool())
		{
			didWork = true;
			transmitPool(pool); // transmit to networkchannels

			// transmit to all other threads
			for (size_t j = 0; j < mThreadCount; ++j)
			{
				if (j != i)
				{
					BaseEventPool* poolClone = requestPool();
					pool->copyTo(poolClone);
					mEventChannelList[j]->receiveEventPool(poolClone);
				}
			}

			// we don't need this pool anymore
			freePool(pool);
		}
	}
	return didWork;
}

void EventManager::publishNetworkPool(BaseEventPool* pool)
{
	boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);

	if (mThreadCount > 0)
	{
		BaseEventPool* poolClone = requestPool();
		pool->copyTo(poolClone);
		mEventChannelList[0]->receiveEventPool(poolClone);
	}
	freePool(pool);
}

long EventManager::getDynamicEventId(const std::string& eventIdentifier)
{
	long wRet = 0;
	boost::mutex::scoped_lock scoped_lock(mDynamicEventIdMapMutex);

	EventIdBinding::iterator it = mDynamicEventIdMap.find(eventIdentifier);
	if (it == mDynamicEventIdMap.end())
	{
		mDynamicEventIdMap[eventIdentifier] = EventIdDynamicStartValue;
		wRet = EventIdDynamicStartValue;
		++EventIdDynamicStartValue;		
	}
	else
	{
		wRet = (*it).second;
	}

	return wRet;
}

int EventManager::getChannelId(EventChannel* ioChannel, bool should_lock) 
{ 
	if (should_lock)
	{
		boost::mutex::scoped_lock scoped_lock(mEventChannelListMutex);
	}

	int wRet = 0; 
	while(mEventChannelList[wRet] != NULL) 
	{ 
		if (mEventChannelList[wRet] == ioChannel)
		{
			return wRet;
		}
		++wRet;
	}
	return -1;
}

// Pool Operations

BaseEventPool* EventManager::requestPool()
{
	BaseEventPool* wRet = NULL;

	boost::recursive_mutex::scoped_lock scoped_lock(mEventPoolMutex);

	if (mPoolQueue.empty())
	{
		wRet = new BaseEventPool();
	}
	else
	{
		wRet = mPoolQueue.front();
		mPoolQueue.pop_front();
	}
	return wRet;
}

void EventManager::freePool(BaseEventPool* pool)
{
	boost::recursive_mutex::scoped_lock scoped_lock(mEventPoolMutex);
	
	pool->clear();
	mPoolQueue.push_back(pool);
}

//-----------------------------------------------------------------------------
// Network stuff
void EventManager::listen(const int port)
{
	dbglog << "EventManager: Listening on port " << port << " ...";

    boost::asio::ip::tcp::endpoint ep(boost::asio::ip::tcp::v4(), port);
    
	if (!mAcceptor)
	{
		mAcceptor = new boost::asio::ip::tcp::acceptor(mService, ep);
		startAccept();
	}
	else
	{
		throw std::logic_error("EventManager::init(): Initialized twice!");
	}
}

void EventManager::connect(const std::string& ip, const int port)
{
	dbglog << "EventManager: Connecting to " << ip << ":" << port << " ...";

    boost::asio::ip::address_v4 addr = boost::asio::ip::address_v4::from_string(ip);
    boost::asio::ip::tcp::endpoint ep(addr, port);
	boost::asio::ip::tcp::socket socket(mService);

	EventSystem::ClientChannel::Pointer newConnection =
		EventSystem::ClientChannel::create(mService);

	newConnection->connect(ep);
}

void EventManager::registerConnection(EventSystem::NetworkChannel* conn)
{
	mConnections.push_back(conn);
	std::cout << "EventManager has " << mConnections.size() 
		<< " registered Connections" << std::endl;
}

void EventManager::unregisterConnection(EventSystem::NetworkChannel* conn)
{
	std::vector<EventSystem::NetworkChannel*>::iterator it = mConnections.begin();
	while(it != mConnections.end())
	{
		if (*it == conn)
		{
			mConnections.erase(it);
			return;
		}
		++it;
	}
}

void EventManager::transmitPool(BaseEventPool* pool)
{
	if (!pool)
		return;

	std::ostringstream dataStream;
	boost::archive::text_oarchive archive(dataStream);

	archive & (*pool);

	std::string outData = dataStream.str();

	std::ostringstream headerStream;
	headerStream << std::setw(8)
	             << std::hex
	             << outData.size();
	std::string outHeader = headerStream.str();

	std::vector<EventSystem::NetworkChannel*>::iterator ncIter = mConnections.begin();
	for (; ncIter != mConnections.end(); ++ncIter)
	{
		EventSystem::NetworkChannel::Pointer channel(*ncIter, null_deleter());

		channel->sendPacket(outHeader, outData);
	}

#if defined(_DEBUG) || !defined(NDEBUG)
	// output
	std::cout << "EventManager has sent:" << std::endl 
	          << (*pool) << std::endl;
#endif
}

void EventManager::startAccept()
{
	EventSystem::ServerChannel::Pointer newConnection =
		EventSystem::ServerChannel::create(mService);

	mAcceptor->async_accept
	(
		newConnection->socket(),
		boost::bind
		(
			&EventManager::handleAccept,
			this,
			newConnection,
			boost::asio::placeholders::error
		)
	);
}

void EventManager::handleAccept(EventSystem::NetworkChannel::Pointer newConnection,
								const boost::system::error_code& error)
{
	std::cout << "New Incoming connection" << std::endl;
	if (!error)
	{
		newConnection->startHandShake();
		startAccept();
	}
}
