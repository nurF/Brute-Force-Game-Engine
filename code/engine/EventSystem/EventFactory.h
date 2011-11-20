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

#ifndef BFG_EVENTSYSTEM_EVENT_FACTORY_H_
#define BFG_EVENTSYSTEM_EVENT_FACTORY_H_

#include <Base/CLogger.h>
#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Core/EventPool.h>
#include <EventSystem/Event.h>

namespace BFG {

struct EventCreationEx : std::runtime_error
{
	EventCreationEx() :
		std::runtime_error("Unable to create new pool element"
		                   " for an event.")
	{
	}
};

struct EventFactory
{
	template <typename EventT>
	static EventT* Create(EventLoop *Loop,
	                      typename EventT::ActionT action,
	                      typename EventT::PayloadT payload)
	{
		assert(Loop != NULL);

		EventT* e = new
		(
			static_cast
			<
				EventT*
			>
			(
				Loop->createPoolElement<EventT>()
			)
		) EventT(action, payload);

		check(e);
		
		return e;
	}
	
	template <typename EventT>
	static EventT* Create(EventLoop *Loop,
	                      typename EventT::ActionT action,
	                      typename EventT::PayloadT payload,
	                      typename EventT::DestinationT destination,
	                      typename EventT::SenderT sender)
	{
		assert(Loop != NULL);
	
		EventT* e = new
		(
			static_cast
			<
				EventT*
			>
			(
				Loop->createPoolElement<EventT>()
			)
		) EventT(action, payload, destination, sender);

		check(e);
		
		return e;
	}
	
private:
	template <typename EventT>
	static void check(EventT* e)
	{
		if (e == NULL)
			throw EventCreationEx();
	}
};

} // namespace BFG

#endif
