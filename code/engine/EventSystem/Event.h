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

#ifndef BFG_EVENTSYSTEM_EVENT_H
#define BFG_EVENTSYSTEM_EVENT_H

#include <EventSystem/Event_fwd.h>

#include <EventSystem/Core/EventDefs.h>

namespace BFG {

template 
<
	typename _ActionT,
	typename _PayloadT,
	typename _DestinationT,
	typename _SenderT
>
class Event : public TBaseEvent<_PayloadT>
{
public:
	typedef _ActionT      ActionT;
	typedef _PayloadT     PayloadT;
	typedef _DestinationT DestinationT;
	typedef _SenderT      SenderT;
	
	typedef typename TBaseEvent<_PayloadT>::IdT IdT;

	Event(ActionT ai = static_cast<ActionT>(UNASSIGNED_EVENTID),
	      PayloadT data = PayloadT()) :
	TBaseEvent<PayloadT>(static_cast<EventIdT>(ai), data)
	{
	}
	
	Event(ActionT ai,
	      PayloadT data,
	      DestinationT destination,
	      SenderT sender) :
	TBaseEvent<PayloadT>(static_cast<EventIdT>(ai), data, destination),
	mDestination(destination),
	mSender(sender)
	{
	}
	
	Event* copy()
	{
		return new Event<ActionT, PayloadT, DestinationT, SenderT>(*this);
	}

	DestinationT mDestination;
	SenderT      mSender;
};

} // namespace BFG

#endif
