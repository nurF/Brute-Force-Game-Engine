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

#ifndef __EVENT_FUNCTOR_TABLE_H_
#define __EVENT_FUNCTOR_TABLE_H_

#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Core/EventFunctor.h>


/// \brief Interface class for accessing EvenTabless
class IEventTable
{
public:
	virtual ~IEventTable() {}
  
    // Dispatch an event over all entries
    virtual size_t call(IEvent* event) = 0;
    // Number of stored functors
    virtual long getSize() const = 0;    
    //virtual void AddEntry (BaseEventFunctor*) = 0;
    // Delete an specific entry
    virtual void deleteEntry (void* listener) = 0;
    // Clear all
    virtual void deleteAllEntries() = 0;
};

template < class EventListener, class EventType=IEvent >
class EventFunctorTable : public IEventTable
{
	typedef SpecificEventFunctor<EventListener,EventType>* ConcretePtr;
public:
	EventFunctorTable(size_t maxEvents=MAX_EVENTS) :
	mEventTable(new ConcretePtr[maxEvents]),
	mMaxEvents(maxEvents),
    mEventCounter(0) {}

	~EventFunctorTable()
	{
		// Free all functors
		for (size_t i = 0; i < mEventCounter; ++i)
		{
			ConcretePtr fctr = mEventTable[i];
			delete (fctr);
		}
        // free
		delete[] mEventTable;
	}

    /// \return The number of calls done by this function
	size_t call(IEvent* event)
	{
        // :TODO: BOOST_IS_TYPE_OF ...
	    // Always assume its the correct event type
		EventType* realType = static_cast<EventType*>(event);
		for (size_t i = 0; i < mEventCounter; ++i)
			mEventTable[i]->call(realType);
	
        // This may count wrong, if some calls are blocked due ReceiverID
		return mEventCounter;
	}

	long getSize() const {return mEventCounter;}

	//template<class EventListener, class EventType>
	//void AddEntry( SpecificEventFunctor<EventListener,EventType> * EventEntry )
	void addEntry(ConcretePtr eventEntry)
	{
		if (mEventCounter > mMaxEvents)
		{
			delete eventEntry;
		}
		else
		{
			mEventTable[mEventCounter] = (ConcretePtr) eventEntry;
			//SpecificEventFunctor<EventListener, EventType> iFctor = m_EventTable [m_EventCounter];
			
			++mEventCounter;
		}
	}

	void deleteEntry(void* listener)
	{
		size_t i = 0;
		while(i < mEventCounter)
		{
			if (mEventTable[i]->getEventListener() == listener )
			{
				// dont change !
				mEventCounter--;
				// Watch out! One specific EventListener could be subscribed with multiple Functions to this event-id
				// This behaviour is not forbidden so far
				// We have to find all Entries, so keep searching and dont break here !
				size_t j = mEventCounter;
				// Copy Last Entry to current position
				delete mEventTable[i];
				mEventTable[i] = mEventTable[j];
			}
			else
			{
				++i;
			}
		}
	}

	void deleteAllEntries()
	{
		for (size_t i = 0; i < mEventCounter; ++i)
		{	
			delete mEventTable[i];
		}
		mEventCounter = 0;
	}
private:
	ConcretePtr* mEventTable;
	size_t	  mMaxEvents;
	size_t	  mEventCounter;
};

#endif //__EVENT_FUNCTOR_TABLE_H_
