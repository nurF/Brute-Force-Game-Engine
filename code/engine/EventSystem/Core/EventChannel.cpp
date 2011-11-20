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
//
//! \todo The EventChannel is not fully develeoped yet. Some parts need to be beautified
//

#include <EventSystem/Core/EventChannel.h>
#include <EventSystem/Core/EventManager.h>


EventChannel::EventChannel() :
mUniqueID(-1)
{
}

EventChannel::~EventChannel()
{
	//! \note Doesn't this leave the events subscribed?
	cleanUpChannel();
}

void EventChannel::cleanUpChannel()
{
	boost::mutex::scoped_lock scoped_lock(mOutgoingMutex);
	
	while(!mOutgoingQ.empty())
	{
		BaseEventPool* pool = mOutgoingQ.front();
		EventManager::getInstance()->freePool(pool);
		mOutgoingQ.pop();
	}

	boost::mutex::scoped_lock scoped_lock2(mIncomingMutex);

	while(!mIncomingQ.empty())
	{
		BaseEventPool* pool = mIncomingQ.front();
		EventManager::getInstance()->freePool(pool);
		mIncomingQ.pop();
	}
}

void EventChannel::deinit()
{
	if (EventManager::isInitialized())
	{
		EventManager::getInstance()->unsubscribeAll(this);
	}
	cleanUpChannel();
}

// --------------------------------------------------
// Publishing Events
// --------------------------------------------------

// EventPool - Method
void EventChannel::publishEventPool(BaseEventPool* pool)
{
	boost::mutex::scoped_lock scoped_lock(mOutgoingMutex);

	appendEventPool(mOutgoingQ, pool);
}

void EventChannel::appendEventPool(EventPoolQueue& queue, BaseEventPool* pool)
{
	// no need to append a pool if the queue is empty
	if (queue.empty())
	{
		queue.push(pool);
		return;
	}
	
	// fill the lastPool till it is full and...
	BaseEventPool* lastPool = mOutgoingQ.back();
	lastPool->append(pool);

	// leave the rest in pool (if there is a rest)
	if (pool->size())
	{
		queue.push(pool);
	}
	else
	{
		EventManager::getInstance()->freePool(pool);
	}
}

BaseEventPool* EventChannel::getPublishedEventPool()
{
	return getFirstPool(mOutgoingQ, mOutgoingMutex);
}

void EventChannel::receiveEventPool(BaseEventPool* pool)
{
	boost::mutex::scoped_lock scoped_lock(mIncomingMutex);

	mIncomingQ.push(pool);
}

BaseEventPool* EventChannel::getReceivedEventPool()
{
	return getFirstPool(mIncomingQ, mIncomingMutex);
}

BaseEventPool* EventChannel::getFirstPool(EventPoolQueue& queue, boost::mutex& locker)
{
	boost::mutex::scoped_lock scoped_lock(locker);

	if (queue.empty())
		return NULL;

	BaseEventPool* pool = queue.front();
	queue.pop();
	return pool;
}

void EventChannel::subscribe(long eventID)
{
	boost::mutex::scoped_lock scopedLock(mSubscriptionMutex);

	//! \note This doesn't cover unsubscribed events
	if (mSubscriptionMap.find(eventID) == mSubscriptionMap.end())
	{
		mSubscriptionMap[eventID] = true;
		EventManager::getInstance()->subscribeEvent(eventID, this);
	}
}

void EventChannel::unsubscribe(long eventID)
{
	boost::mutex::scoped_lock scopedLock(mSubscriptionMutex);

	SubcriptionMapT::iterator it = mSubscriptionMap.find(eventID);
	if (it != mSubscriptionMap.end())
	{
		if (it->second)
		{
			mSubscriptionMap[eventID] = false;	
			//! \bug Unsubscribe only once ... maybe a bug
			EventManager::getInstance()->unsubscribeEvent(eventID, this);
		}
	}
}
