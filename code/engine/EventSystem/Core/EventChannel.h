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
// The Event-Channel secures the Communication between EventLoops and the EventManager
// There is exactly one EventChannel for each EventLoopThreaded, all Events beeing transmitted
// are queued within this class most of the time.
//
// Incoming means : Communication from the EventManager to the EventLoop
// Outgoing means : Communication from the EventLoop to the EventManager
// To sum this up, proper communication needs the correct usage of :
// PublishEvents + GetReceivedEvents ( EventLoop )
// GetPublishEvents + ReceiveEvents  ( EventManager )
//
// Remember! This will only be used in a threaded environment and you never work directly with this class.
// Remember! The EventChannel is always passive
//
//! \todo The EventChannel is not fully develeoped yet. Some parts need to be beautified
//

#ifndef __EVENT_CHANNEL_H_
#define __EVENT_CHANNEL_H_

#include <deque>
#include <vector>
#include <queue>

#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Core/EventPool.h>

//! \todo fix all typedefs, move to EventDefs.h

typedef std::deque<IEvent*> EventDeque;
typedef std::queue<IEventPtr*> EventArrayQueue;

typedef std::queue<BaseEventPool*> EventPoolQueue;

class EventChannel
{
public:
	EventChannel();
	~EventChannel();
	// Before deleting the EventChannel itself, use the DeInit-Function !
	void deinit();
	//! \todo Do proper Comment for CleanUpChannel()
	void cleanUpChannel();
	// Event-Subbscription Mechanism	
	void subscribe(long eventID);
	void unsubscribe(long eventID);

	// --------------------------------------------------
	// Communication Interface
	// :NOTE: This Interface is under construction, so look in the implementation first
	// --------------------------------------------------
	// EventPool - Method
	void publishEventPool(BaseEventPool* pool);
	BaseEventPool* getPublishedEventPool();

	void receiveEventPool(BaseEventPool* pool);
	BaseEventPool* getReceivedEventPool();
	// --------------------------------------------------

	// --------------------------------------------------
	// Currently it is uncertain, if this UniqueID is really needed
	// Atm. it is only used by the EventManager, to correctly identify the EventChannels ( pointers dont have names )
	long getUniqueId() const {return mUniqueID;}
	void setUniqueId(long id) {mUniqueID = id;}

	size_t getIncomingSize() {return mIncomingQ.size();}
	size_t getOutgoingSize() {return mOutgoingQ.size();}
private:
	BaseEventPool* getFirstPool(EventPoolQueue& q, boost::mutex& locker);

	void appendEventPool(EventPoolQueue& queue, BaseEventPool* pool);

	// The Unique-ID,  only needed by EventManager
	long mUniqueID;

	// The Suscription Map
	typedef std::map<long, bool> SubcriptionMapT;
	SubcriptionMapT mSubscriptionMap;

	EventPoolQueue mIncomingQ;
	EventPoolQueue mOutgoingQ;

	// Mutex, for locking the local shared data
	boost::mutex mIncomingMutex;
	boost::mutex mOutgoingMutex;
	boost::mutex mSubscriptionMutex;

};

#endif //__EVENT_CHANNEL_H_
