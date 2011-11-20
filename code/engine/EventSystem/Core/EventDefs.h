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

#ifndef __EVENT_DEFS_H_
#define __EVENT_DEFS_H_

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32)
#define WIN32_LEAN_AND_MEAN
#endif 

#include <EventSystem/Core/EventHeader.h>
#include <EventSystem/Core/EventIds.h>


//! \attention
//!		Please use this type with high caution. Check first if
//!		TBaseEvent<>::IdT or Event<>::IdT suit your needs.
typedef long EventIdT;

static const EventIdT UNASSIGNED_EVENTID = 0;

//! Number of connected callbacks
static const unsigned long MAX_EVENT_LISTENERS = 256;

//! Limit of queued events per cycle
static const unsigned long MAX_EVENTS = 4000;

typedef LocalEventHeader IEvent;
typedef IEvent* IEventPtr;

template < typename DataType, typename Header = IEvent >	
class TBaseEvent : public Header
{
public:
	typedef typename Header::IdT         IdT;
	typedef typename Header::ReceiverIdT ReceiverIdT;

	// Protected, because we never use this class directly, except from derived classes
	//TBaseEvent():TBaseEvent():DataType(){};
	TBaseEvent(void) {}
	TBaseEvent(const typename Header::IdT id): Header(id, 0), mData() {}
	TBaseEvent(const typename Header::IdT id, const DataType& data, const ReceiverIdT receiver = 0) : 
		Header(id, receiver), mData(data){}

	virtual ~TBaseEvent(){}
	// Base Interface
	//inline const DataType& getData() const { return m_Data; }
	// changed to non-const, to avoid problems
	DataType& getData() {return mData;}
	const DataType& getData() const {return mData;}
	void setData(const DataType& data) {mData = data;}

	// BaseEvent can not Copy itself
	virtual bool canCopy() const {return true;}
	virtual inline TBaseEvent* copy() 
	{ 
		TBaseEvent* wRet = new TBaseEvent(Header::mId, getData(), Header::mReceiver); 
		return wRet; 
	}

	virtual void copy(TBaseEvent* newPlace)
	{
		newPlace->setData(getData());
		newPlace->setId(Header::getId());
		newPlace->setReceiver(Header::getReceiver());
	}

	operator const DataType&() const {return mData;}

	friend std::ostream& operator << (std::ostream& os, TBaseEvent& ev)
	{
#ifdef EVENTSYSTEM_NETWORK_TEST
		os << "TBaseEvent(" << ev.mId << ", " << ev.mReceiver << ")" 
			<< "Data (" << ev.mData 
			<< ")" << std::endl;
#endif
		return os;
	}

	virtual unsigned int getSize(void) const
	{
		return sizeof(TBaseEvent<DataType, Header>);
	}

	TBaseEvent(const TBaseEvent& event) : mData(event.mData) {} 


private:
	TBaseEvent operator=(const TBaseEvent&);// {};
	DataType mData;
};

typedef IEvent BaseEvent;

#endif //__EVENT_DEFS_H_
