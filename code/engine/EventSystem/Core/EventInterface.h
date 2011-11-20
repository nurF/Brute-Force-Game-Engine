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

#ifndef __EVENT_INTERFACE_H_
#define __EVENT_INTERFACE_H_

#include <map>
#include <vector>

#include <boost/array.hpp>

#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Core/EventPool.h>
#include <EventSystem/Core/EventFunctorTable.h>


#if !defined(NDEBUG)
//#define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
//#define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
#endif

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/composite_key.hpp>


struct EventBinding
{
	long long mEventId;
	long long mReceiver; // handle
	IEventTable* mTable;

	EventBinding(long long id, long long r, IEventTable* t) :
	mEventId(id), mReceiver(r), mTable(t) {}
};

//! The Events will be stored in List of type EventListArray
typedef boost::array<BaseEvent*,MAX_EVENTS> EventListArray;

typedef boost::multi_index_container
<
	EventBinding, 
	boost::multi_index::indexed_by
	<
		boost::multi_index::ordered_unique
		<
			boost::multi_index::composite_key
			< 
				EventBinding,
				boost::multi_index::member
				<
					EventBinding,
					long long,
					&EventBinding::mEventId
				>,
				boost::multi_index::member
				<
					EventBinding,
					long long,
					&EventBinding::mReceiver
				>
			>
		>
	>
> MultiIndexEventMapType;


class IEventProcessor
{
public:
	virtual ~IEventProcessor() {}

private:
	virtual void processEvent(BaseEvent* event) = 0;
	virtual void processEvents(BaseEventPool* eventPool) = 0;

	//	template<typename EventListener>
	//	void Connect( long EventId, EventListener * Listener, void(EventListener::*FunctionPointer)(BaseEvent *event)  ) = 0;
	virtual void disconnect(long eventId, void* listener) = 0;
};


class EventProcessor : public IEventProcessor
{
public:
	EventProcessor();
	virtual ~EventProcessor();

	virtual void processEvent(BaseEvent* event);
	virtual void processEvents(BaseEventPool* eventPool);

	virtual long processEventCount(BaseEvent* event);
	virtual long processEventsCount(BaseEventPool* eventPool, size_t startCount);

	template < class EventListener, class EventType >
	void connect(long eventId, 
                 EventListener* listener,
                 void(EventListener::*functionPointer)(EventType* event),
                 long long receiverID = 0)
	{
		SpecificEventFunctor<EventListener,EventType>* fct = 
			new SpecificEventFunctor<EventListener, EventType>
			(
				listener,
				functionPointer,
				receiverID
			);

		addEventListener(eventId, fct, receiverID);
	}
	
	void disconnect(long eventId, void* listener);


private:
	MultiIndexEventMapType mEventMap;

protected:
	template<class EventListener, class EventType>
	void addEventListener(long eventId,
                          SpecificEventFunctor<EventListener, EventType>* eventEntry = NULL,
                          long long receiverID = 0)
	{
		if (eventEntry == NULL)
		{
			throw std::logic_error("EventInterface::addEventListener: no listener named!");
		}

		IEventTable* wTable = NULL;
		MultiIndexEventMapType::const_iterator it = 
			mEventMap.find(boost::make_tuple(eventId, receiverID));
		
		if (mEventMap.end() == it)
		{
			// If not found, create a new EventTable for this (specific) event
			wTable = new EventFunctorTable<EventListener, EventType>(MAX_EVENT_LISTENERS);
			mEventMap.insert(EventBinding(eventId, receiverID, wTable));
		}
		else
		{
			wTable = it->mTable;
		}

		//! \todo Add assertion and compile-time-checks like, is_same_class etc.
		EventFunctorTable<EventListener, EventType>* hurz = 
			static_cast<EventFunctorTable<EventListener, EventType>*> (wTable);
		hurz->addEntry(eventEntry);
	};

	void cleanUp();
};

#endif // __EVENT_INTERFACE_H_

