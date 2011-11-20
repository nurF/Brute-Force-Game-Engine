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

#ifndef __EVENT_THREADING_POLICY_H_
#define __EVENT_THREADING_POLICY_H_

#include <boost/thread/thread.hpp>

#include <EventSystem/Core/IEventLoop.h>


namespace EventSystem
{

typedef void(*DefaultEntryPointSignature)(IEventLoop*);
typedef void(*NamedEntryPointSignature)(const std::string&, IEventLoop*);

// Of course this seems to be a hack
extern void eventEntryHelper(IEventLoop* loop);
extern void eventEntryHelperNamed(const std::string& name, IEventLoop* loop);

//! \brief General Interface for threading policies
class ThreadingPolicy
{
public:
	ThreadingPolicy() {}
	virtual ~ThreadingPolicy() {}
	
	virtual void start(IEventLoop*) = 0;
	virtual void stop() = 0;
};

//! \brief Dummy implementation for threading
template <typename EntryPointT = DefaultEntryPointSignature>
class NoThread : public ThreadingPolicy
{
public:
	NoThread(EntryPointT ep = eventEntryHelper) : 
		ThreadingPolicy(),
		mEntry(ep)
	{}

	virtual void start(IEventLoop* loop)
	{
		(*mEntry)(loop);
	}

	virtual void stop()
	{}

private:  
	EntryPointT mEntry;  
};

//! \brief Boost Implementation for threading
template <typename EntryPointT = NamedEntryPointSignature>
class BoostThread : public ThreadingPolicy
{
public:
	BoostThread(const std::string& name, EntryPointT ep = eventEntryHelperNamed) : 
		ThreadingPolicy(),
		mName(name),
		mEntry(ep)
	{}

	virtual void start(IEventLoop* loop)
	{
		mThread = boost::thread(mEntry, mName, loop);
	}

	virtual void stop()
	{
		mThread.join();
	}

private:
	std::string mName;
	EntryPointT mEntry;

	boost::thread mThread;
};

} // namespace EventSystem

#endif //__EVENT_THREADING_POLICY_H_
