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

#ifndef __EVENT_MANAGER_H_
#define __EVENT_MANAGER_H_

#include <string>
#include <map>
#include <queue>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>

#include <Base/CSingleton.h>

#include <EventSystem/Core/EventPool.h>
#include <EventSystem/Core/EventChannel.h>

#include <EventSystem/Core/network/NetworkChannel.h>


class EventLoop;
class EventManager;

//! \todo Do assigned of static in cpp-file
//! Causes trouble with multiple includes 

static const int max_threads = 32;
static long EventIdDynamicStartValue = 30000;

// EventChannel-Typedef : EventPoolQueue
typedef std::map<long, long> EventChannelMaskMap;
typedef std::map<std::string, long> EventIdBinding;


class EventManager
{
public:
	//! Friend needed for Base::CSingleton to work
	friend class BFG::Base::CLazyObject;
	//! cmon its a singleton
	static EventManager* getInstance();
	//! Destructor needs to be public
	~EventManager();
	//! cmon its a singleton
	//static void Init () { m_Self = new EventManager(); }
	//! Thats reallly brutal
	static void destroy()
	{
		//if (IsInitialized())
		//delete m_Self; 
		//m_Self = NULL; 
	}
	static bool isInitialized()
	{
		return true;
	}
	//static bool DoTick() { return GetInstance()->DoOperate(); }

	long registerEventLoop(EventChannel* ioChannel);
	void unregisterEventLoop(EventChannel* ioChannel);
	void subscribeEvent(long eventId, EventChannel* ioChannel);
	void subscribeEvents(long* eventId, EventChannel* ioChannel);
	void unsubscribeEvent(long eventId, EventChannel* ioChannel);
	void unsubscribeAll(EventChannel* ioChannel);

	long getDynamicEventId(const std::string& eventIdentifier);

	int getChannelId(EventChannel* ioChannel, bool should_lock = true);

	//! EventPool-Operations
	BaseEventPool* requestPool();
	void freePool(BaseEventPool* pool);

	// Network stuff
	// Relocate this to proper policy ?
	void listen(const int port);
	void connect(const std::string& ip, const int port);

	void registerConnection(EventSystem::NetworkChannel* conn);
	void unregisterConnection(EventSystem::NetworkChannel* conn);

	void transmitPool(BaseEventPool* pool);
	void publishNetworkPool(BaseEventPool* pool);

private:
	//! Ctor is only called by Singleton-creation function
	EventManager(void);
	//! Is exchanging Inter-Process data here
	bool doOperate();
	//! Entrypoint for the thread
	void runThread();
	//! The "worker" thread
	boost::thread	mThread;
	// Determine how many locks actually are needed
	boost::mutex mInternalMutex;
	boost::mutex mEventChannelListMutex;
	boost::mutex mExecutionListMutex;
	boost::mutex mDynamicEventIdMapMutex;

	boost::recursive_mutex mEventPoolMutex;
	size_t mThreadCount;
	//std::vector<EventChannel*> m_ChannelList;
	//EventChannelMMap m_ExecutionList;
	EventChannelMaskMap mExecutionList;
	EventChannel* mEventChannelList[max_threads];
	EventIdBinding mDynamicEventIdMap;
	//EventPoolQueue m_PoolQueue;
	std::deque<BaseEventPool*> mPoolQueue;
	//! Condition for shutting down the EventSystem
	bool mExitFlag;

	//-------------------------------------------------------------------------
	// Network stuff
	// Relocate this to proper policy ?
	boost::asio::io_service mService;
	boost::asio::ip::tcp::acceptor* mAcceptor;

	void startAccept();

	void handleAccept(EventSystem::NetworkChannel::Pointer new_connection,
		const boost::system::error_code& error);

	//! All the currently connected network connections
	std::vector<EventSystem::NetworkChannel*> mConnections;
	//boost::asio::ip::tcp::socket* _socket;
};

#endif // __EVENT_MANAGER_H_
