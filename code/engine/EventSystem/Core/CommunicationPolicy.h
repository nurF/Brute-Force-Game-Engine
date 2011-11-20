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

#ifndef EVENT_COMMUNICATION_POLICY_H
#define EVENT_COMMUNICATION_POLICY_H

#include <boost/scoped_ptr.hpp>

#include <EventSystem/Core/EventPool.h>

class EventChannel;

namespace EventSystem
{

class CommunicationPolicy
{
public:
	CommunicationPolicy(const int& id = 0) : mId(id) {}
	virtual ~CommunicationPolicy() {}

	int id() const {return mId;}
	void id(const int& ID) {mId = ID;}

	virtual void init() = 0;
	virtual void deinit() = 0;      
	virtual void publishPool(BaseEventPool*) = 0;
	virtual BaseEventPool* receivePool() = 0;
private:
	int mId;	
};


class NoCommunication : public CommunicationPolicy
{
public:
	NoCommunication() : CommunicationPolicy() {}
	virtual ~NoCommunication() {}

	virtual void init() {}
	virtual void deinit() {}  
	virtual void publishPool(BaseEventPool*) {}
	virtual BaseEventPool* receivePool() {return 0;}
};


class InterThreadCommunication : public CommunicationPolicy
{
public:
	InterThreadCommunication();
	virtual ~InterThreadCommunication();

	virtual void init();
	virtual void deinit();
	virtual void publishPool(BaseEventPool*);
	virtual BaseEventPool* receivePool();

private:
	EventChannel* mIoChannel;
};

} // namespace EventSystem

#endif //__EVENT_COMMUNICATION_POLICY_H_
