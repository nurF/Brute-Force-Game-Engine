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

#include <Controller/Controller.h>

#include <Base/CLogger.h>
#include <Core/ClockUtils.h>
#include <EventSystem/Core/EventLoop.h>
#include <Controller/State.h>
#include <Controller/ControllerEvents.h>

namespace BFG {
namespace Controller_ {

namespace posix_time = boost::posix_time;

//#############################################################################
//                        The almighty Controller
//#############################################################################

Controller::Controller(EventLoop* loop) :
mEventLoop(loop),
mWindowWidth(0),
mWindowHeight(0)
{
	assert(loop && "Controller: EventLoop not initialized!");
	
	loop->registerLoopEventListener(this, &Controller::loopHandler);
}

Controller::~Controller()
{
	mEventLoop->unregisterLoopEventListener(this);

	int first = ID::CE_FIRST_CONTROLLER_EVENT + 1;
	int last = ID::CE_LAST_CONTROLLER_EVENT;
	
	for (int i=first; i < last; ++i)
	{
		mEventLoop->disconnect(i, this);
	}
}

void Controller::init(boost::shared_ptr<OIS::InputManager> aInputManager,
                      int maxFrameratePerSec,
                      u32 windowWidth,
                      u32 windowHeight)
{
	dbglog << "Controller: Initializing with "
	       << maxFrameratePerSec << " FPS";

	int first = ID::CE_FIRST_CONTROLLER_EVENT + 1;
	int last = ID::CE_LAST_CONTROLLER_EVENT;

	for (int i=first; i < last; ++i)
	{
		mEventLoop->connect(i, this, &Controller::controlHandler);
	}

	if (! aInputManager)
		throw std::logic_error("Controller: OIS::InputManager invalid.");
	
	if (maxFrameratePerSec <= 0)
		throw std::logic_error("Controller: Invalid maxFrameratePerSec param");

	mClock.reset
	(
		new Clock::SleepFrequently
		(
			Clock::microSecond,
			ONE_SEC_IN_MICROSECS / maxFrameratePerSec
		)
	);

	mInputSystem = aInputManager;
	mWindowWidth = windowWidth;
	mWindowHeight = windowHeight;
}

void Controller::insertState(StateInsertion& si)
{
	std::string name            = si.mStateName.c_array();
	std::string config_filename = si.mConfigurationFilename.c_array();

	dbglog << "Controller: Inserting state \"" << name << "\"";

	boost::shared_ptr<State> state(new State(mEventLoop));

	mStates.insert(std::make_pair(si.mHandle, state));

	state->init
	(
		std::string(name),
		std::string(config_filename),
		mActions,
		mInputSystem,
		mWindowWidth,
		mWindowHeight
	);

	if (si.mActivateNow)
		activateState(si.mHandle);
}

void Controller::removeState(GameHandle state)
{
	dbglog << "Controller: Removing State \"" << state << "\"";

	StateContainerT::iterator it = mStates.find(state);

	if (it != mStates.end())
	{
		mStates.erase(it);
	}
}

void Controller::capture()
{
	StateContainerT::iterator it = mStates.begin();
	for (; it != mStates.end(); ++it)
		it->second->capture();
}

void Controller::sendFeedback(long microseconds_passed)
{
	StateContainerT::iterator it = mStates.begin();
	for (; it != mStates.end(); ++it)
		it->second->sendFeedback(microseconds_passed);
}

void Controller::activateState(GameHandle state)
{
	dbglog << "Controller: Activating State \"" << state << "\"";

	StateContainerT::const_iterator it = mStates.find(state);

	if (it == mStates.end())
		errlog << "Controller: Unable to activate state \"" << state << "\"";

	it->second->activate();
}

void Controller::deactivateState(GameHandle state)
{
	dbglog << "Controller: Deactivating State \"" << state << "\"";

	StateContainerT::const_iterator it = mStates.find(state);

	if (it == mStates.end())
		errlog << "Controller: Unable to deactivate state \"" << state << "\"";

	it->second->deactivate();
}

void Controller::addAction(const ActionDefinition& ad)
{
	mActions.insert
	(
		std::make_pair
		(
			ad.get<0>(),
			std::string(ad.get<1>().data())
		)
	);
}

void Controller::loopHandler(LoopEvent*)
{
	nextTick();
}

void Controller::controlHandler(ControlEvent* e)
{
	switch(e->getId())
	{
		case ID::CE_ADD_ACTION:
			addAction(boost::get<ActionDefinition>(e->getData()));
			break;
	
		case ID::CE_LOAD_STATE:
			insertState(boost::get<StateInsertion>(e->getData()));
			break;

		case ID::CE_ACTIVATE_STATE:
			activateState(boost::get<GameHandle>(e->getData()));
			break;
			
		case ID::CE_DEACTIVATE_STATE:
			deactivateState(boost::get<GameHandle>(e->getData()));
			break;
	};
}

void Controller::nextTick()
{
	capture();
	sendFeedback(mClock->measure());
}

void Controller::resetInternalClock()
{
	mClock->reset();
}

} // namespace Controller_
} // namespace BFG
