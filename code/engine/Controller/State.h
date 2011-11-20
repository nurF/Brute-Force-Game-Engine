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

#ifndef _STATE_H_
#define _STATE_H_

#include <set>
#include <boost/shared_ptr.hpp>

#include <Controller/Action.h>
#include <Controller/OISDevice.h>

class EventLoop;

namespace BFG {
namespace Controller_ {

class State
{
public:	
	explicit State(EventLoop* loop);
	~State();

	void init(const std::string& state,
	          const std::string& configFilename,
	          const ActionMapT& actions,
	          boost::shared_ptr<OIS::InputManager>,
	          u32 windowWidth,
	          u32 windowHeight);

	void capture();

	void sendFeedback(long microseconds_passed);

	EventLoop* eventLoop() const;

	const std::string& getStateID() const;

	void activate();
	void deactivate();

private:
	void visitFeedback(VipPtrT Vip);
	void disableFeedbackOfAllVips();

	typedef std::set<VipPtrT> FeedbackT;

	std::string mID;

	Devices::OISMouse    mMouse;
	Devices::OISKeyboard mKeyboard;
	Devices::OISJoystick mJoystick;

	FeedbackT mFeedbacks;

	EventLoop* mEventLoop;
};

} // namespace Controller_
} // namespace BFG


#endif
