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

#include <EventSystem/Core/EventInterface.h>

EventProcessor::EventProcessor()
{
}

EventProcessor::~EventProcessor()
{
	cleanUp();
}

void EventProcessor::processEvent(BaseEvent* event)
{
	// Doing a double search-operation here ?
	// check complexity for find and operator[]

	MultiIndexEventMapType::const_iterator it = mEventMap.find
	(
		boost::make_tuple(event->getId(), event->getReceiver())
	);

	if (mEventMap.end() != it)
	{
		IEventTable * wTable = it->mTable;
		//size_t num_Events =
		wTable->call(event);
	}
}

void EventProcessor::processEvents(BaseEventPool* eventPool)
{
	size_t eventCounter = eventPool->size();
	size_t i = 0;
	while(i < eventCounter)
	{
		processEvent((*eventPool)[i]);
		++i;
	}
}

long EventProcessor::processEventCount(BaseEvent* event)
{
	long numEvents = 0;
	// Doing a double search-operation here ?
	// check complexity for find and operator[]
	//EventMapType::iterator iter = m_EventMap.find(EventId);
	MultiIndexEventMapType::const_iterator it = mEventMap.find
	(
		boost::make_tuple(event->getId(), event->getReceiver())
	);

	if (it != mEventMap.end())
	{
		IEventTable* wTable = it->mTable;
		numEvents = wTable->call(event);
	}
#ifdef DEBUG_EVENT_ACCEPTANCE
	else
	{
		std::cout << "No recipient for eventId:" << event->getId()
		          << " with destination: " << event->getReceiver()
		          << std::endl;
	}
#endif
	
	return numEvents;
}

long EventProcessor::processEventsCount(BaseEventPool* eventPool, size_t startCount)
{
	long overallEventCount = 0;
	size_t eventCounter = eventPool->size();
	size_t i = startCount;
	while(i < eventCounter)
	{
		overallEventCount += processEventCount((*eventPool)[i]);
		++i;
	}
	return overallEventCount;
}

void EventProcessor::disconnect(long eventId, void* listener)
{
	MultiIndexEventMapType::iterator begin = mEventMap.lower_bound(eventId);
	MultiIndexEventMapType::iterator end = mEventMap.upper_bound(eventId);
	while (begin != end)
	{
		IEventTable* wTable = begin->mTable;
		wTable->deleteEntry(listener);
		if (wTable->getSize() == 0)
		{
			delete wTable;
			mEventMap.erase(begin);
			begin = mEventMap.lower_bound(eventId);
			// 'begin' could be 'end' right here and we would enter garbage area
			// behind 'end' if we'd increment 'begin' now.
		}
		else
			++begin;
	}
}

void EventProcessor::cleanUp()
{
	MultiIndexEventMapType::iterator store_iter = mEventMap.begin();

	while(store_iter != mEventMap.end())
	{
		IEventTable* wTable = store_iter->mTable;
		wTable->deleteAllEntries();
		delete wTable;
		++store_iter;
	}
	mEventMap.clear();
}
