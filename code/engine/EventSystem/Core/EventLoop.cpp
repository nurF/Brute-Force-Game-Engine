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

#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Core/EventManager.h>

EventLoop::EventLoop(bool notifyLoopEventListener, EventSystem::ThreadingPolicy* tp, EventSystem::CommunicationPolicy* cp) :
IEventLoop (),
EventProcessor(),
BFG::Base::CEntryPointHandler<EventLoop>(),
mMultiLoop(notifyLoopEventListener), 
mShouldExit(false),
mFrontPool(new BaseEventPool()),
mBackPool(new BaseEventPool()),
mThreadingPolicy(tp),
mCommunicationPolicy(cp) {}

EventLoop::~EventLoop()
{
	delete mFrontPool;
	delete mBackPool;
}

void EventLoop::run()
{
	mThreadingPolicy->start(this);
}

void EventLoop::stop()
{
	mShouldExit = true;
}

void EventLoop::entry()
{
	mCommunicationPolicy->init();
	if ( !hasEntryPoints() )
	{
		throw std::logic_error("EventLoop: This loop has no EntryPoints!");
	}
	// Run synchroniously
	callEntryPoints(this);
	while(!mShouldExit)
		doLoop();

//	mCommunicationPolicy->deinit();

	mThreadingPolicy->stop();
}

long EventLoop::doLoop()
{
	if (mMultiLoop)
	{
		// Notify our LoopListeners, they want to know, whats up with the Loop
		LoopEvent* ev = new LoopEvent(this, 0);
		mLoopEventReceivers.call(ev);
		delete ev;
	}	

	long eventCount = 0;
	BaseEventPool* incoming = 0;

	// While Events are stored
	do 
	{
		while (mFrontPool->size() > 0)
		{
			// 1. Switch Buffers
			std::swap(mFrontPool, mBackPool);

			// 2. Process BackPool
			eventCount += processEventsCount(mBackPool, 0);

			// 2.1. Publish BackPool
			mCommunicationPolicy->publishPool(mBackPool);

			mBackPool->clear();
		}	

		if (incoming && incoming->size() > 0)
		{
			eventCount += processEventsCount(incoming, 0);
			EventManager::getInstance()->freePool(incoming);
		}
		incoming = mCommunicationPolicy->receivePool();
	} 
	while(incoming);

	return eventCount;
}

void EventLoop::executeEventSyncron(BaseEvent* event)
{
	// Threads and so on ...
	//ExecuteEvent(event->getId(), event);
	assert(false);
}

void EventLoop::cleanUpEventSystem()
{
	//EventQueue::SafeDeleteStoredEvents();
	mFrontPool->clear();
	mBackPool->clear();
	EventProcessor::cleanUp();
}
