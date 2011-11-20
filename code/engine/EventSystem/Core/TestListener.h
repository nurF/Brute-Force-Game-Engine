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

#ifndef __EVENT_TEST_LISTENER_H_
#define __EVENT_TEST_LISTENER_H_

#include <cmath>

#include <EventSystem/Core/EventDefs.h>


template < typename EventType >
class TestListener
{
public:
	TestListener()
	{
		
	}
	~TestListener(){}

	inline void execute(EventType* event)
	{ 
		long* pt = (long*)&(event->getData());
		(*pt)++;
	}
	inline void execute2(EventType* event)
	{
		long* pt = (long*)&(event->getData());
		(*pt)++;
		long j = (*pt);
		for (unsigned int i = 1; i < 100; ++i)
		{
			(*pt) = (long)(sqrt((double)j));
		}
		(*pt) = j;
	}
	//SpecificEventFunctor<BaseEvent>
};
#endif //__EVENT_TEST_LISTENER_H_

