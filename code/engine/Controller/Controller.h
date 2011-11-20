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

#ifndef _CONTROLLER_H_
#define _CONTROLLER_H_

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>

#include <Core/Types.h>

#include <Controller/Action.h>
#include <Controller/ControllerEvents_fwd.h>

namespace OIS
{
	class InputManager;
}

class LoopEvent;
class EventLoop; // Confused?

namespace BFG {

namespace Clock {
	class SleepFrequently;
}

namespace Controller_ {

class State;

class Controller
{
public:
	Controller(EventLoop* loop);
	~Controller();

	void init(boost::shared_ptr<OIS::InputManager> aInputManager,
	          int maxFrameratePerSec,
	          u32 windowWidth,
	          u32 windowHeight);

	void nextTick();

	void resetInternalClock();

	void loopHandler(LoopEvent*);

	void controlHandler(ControlEvent* iLE);

private:
	void capture();
	
	void sendFeedback(long microseconds_passed);

	void insertState(StateInsertion& si);
	void removeState(GameHandle state);

	void activateState(GameHandle state);
	void deactivateState(GameHandle state);

	void addAction(const ActionDefinition&);

	typedef std::map
	<
		GameHandle,
		boost::shared_ptr<Controller_::State>
	> StateContainerT;

	boost::shared_ptr<State>                  mActiveState;
	boost::shared_ptr<OIS::InputManager>      mInputSystem;

	StateContainerT                           mStates;
	ActionMapT                                mActions;

	boost::scoped_ptr<Clock::SleepFrequently> mClock;

	EventLoop*                                mEventLoop;

	u32                                       mWindowWidth;
	u32                                       mWindowHeight;
};

} // namespace Controller_
} // namespace BFG

// For typing convenience
using BFG::Controller_::Controller;

#endif
