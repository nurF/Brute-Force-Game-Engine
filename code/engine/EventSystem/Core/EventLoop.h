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
// Now comes the most important part, our EventLoop
// Since this is the simple, single-threaded BaseClass, dont expect some specialities here
//

#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <Base/CEntryPoint.h>
#include <Base/Compiler.h>

#include <EventSystem/Core/CommunicationPolicy.h>
#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Core/EventInterface.h>
#include <EventSystem/Core/EventPool.h>
#include <EventSystem/Core/IEventLoop.h>
#include <EventSystem/Core/ThreadingPolicy.h>


class EventLoop;

struct EventLoopData
{
	EventLoopData(EventLoop* loop = NULL, long tick = 0, int state = 0 ) : 
	mOrigin(loop),
	mTick(tick),
	mLoopState(state) {} 

	EventLoop* mOrigin;
	long	   mTick;
	int		   mLoopState;
	EventLoop* getLoop() const {return mOrigin;}
};

class LoopEvent : public TBaseEvent<EventLoopData>
{
public:
	LoopEvent(EventLoop* loop = NULL, long tick = 0) :
		TBaseEvent<EventLoopData>(0, EventLoopData(loop, tick)){}
	virtual ~LoopEvent() {}
};


class EventLoop : public EventProcessor, 
                  public IEventLoop,
                  public BFG::Base::CEntryPointHandler <EventLoop>
{
public:

	EventLoop
	(	
		bool notifyLoopEventListener, 
		EventSystem::ThreadingPolicy* tp = new EventSystem::NoThread<>(),
		EventSystem::CommunicationPolicy* cp = new EventSystem::NoCommunication()
	);

	virtual ~EventLoop();
	//! The most used function
	virtual long doLoop();

	//! Don't store it, Execute it immediately
	void executeEventSyncron(BaseEvent *event);

	//! Let it run
	virtual void run();

	//! Stop the Loop
	virtual void stop();

	//! EntryPoint for the Business-Logic
	virtual void entry();

	//! Perform internal Cleanup, proper usage of this function should avoid memory leaks
	virtual void cleanUpEventSystem();

	//! More important in MT-Case
	int getCommunicationID() const {return mCommunicationPolicy->id();}

	template < class EventType >
	EventType* createPoolElement()
	{
		//! \todo boost_is_base_of
		return mFrontPool->reserveNextStorage<EventType>();
	}

	//! More important in MT-Case
	void setExitFlag(bool flag = true) { mShouldExit = flag; }
	bool shouldExit() const { return mShouldExit; }

	template<class EventListener>
	void registerLoopEventListener(
		EventListener* listener,
		void(EventListener::*functionPointer)(LoopEvent* event))
	{
		SpecificEventFunctor<EventListener, LoopEvent>* fct = 
			new SpecificEventFunctor<EventListener, LoopEvent>(listener, functionPointer, 0);
		SpecificEventFunctor<EventLoop, LoopEvent>* func = 
			(SpecificEventFunctor<EventLoop, LoopEvent>*) fct;
		mLoopEventReceivers.addEntry(func);
	}

	void unregisterLoopEventListener(void* listener)
	{
		mLoopEventReceivers.deleteEntry(listener);
	}

private:
	//! \todo Proper Description
	// EventMapType m_EventMap;

	BaseEventPool* mFrontPool;
	BaseEventPool* mBackPool;

	EventSystem::ThreadingPolicy* mThreadingPolicy;
	EventSystem::CommunicationPolicy* mCommunicationPolicy;

	// ----------------------
	//! LoopPoints
	EventFunctorTable<EventLoop,LoopEvent> mLoopEventReceivers;

	//! Flag
	bool mShouldExit;

	bool mMultiLoop;


};

#endif // __EVENT_LOOP_H_
