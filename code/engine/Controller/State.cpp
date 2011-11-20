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

#include <Controller/State.h>

#include <boost/bind.hpp>

#include <Core/Path.h>
#include <Controller/VIPXmlInitializer.h>

namespace BFG {
namespace Controller_ {

void debugOut(VipPtrT vip, const ActionMapT& actions)
{
	const std::string deviceStr = ID::asStr(vip->mDevice);
	
	EventIdT action = vip->getAction();
	ActionMapT::const_iterator it = actions.find(action);
	const std::string actionStr = it == actions.end()?"Unserializable":it->second;
	
	dbglog << deviceStr
	       << " => Action " << action
	       << " (" << actionStr << ")";
}
	
State::State(EventLoop* loop) :
mEventLoop(loop)
{}

State::~State()
{}

void State::init(const std::string& state,
                 const std::string& configFilename,
                 const ActionMapT& actions,
                 boost::shared_ptr<OIS::InputManager> inputManager,
                 u32 windowWidth,
                 u32 windowHeight)
{
	infolog << "Controller: Initializing State \"" << state << "\"";
	mID = state;

	if (actions.empty())
	{
		throw std::logic_error
			("Controller::State: Init of this State is pointless without Actions");
	}

	if (! mKeyboard.init(inputManager, windowWidth, windowHeight))
		warnlog << "Controller: Continuing without the keyboard device.";

	if (! mMouse.init(inputManager, windowWidth, windowHeight))
		warnlog << "Controller: Continuing without the mouse device.";

	if (! mJoystick.init(inputManager, windowWidth, windowHeight))
		warnlog << "Controller: Continuing without the joystick device.";

	if (! (mJoystick.good() || mMouse.good() || mKeyboard.good()))
	{
		throw std::runtime_error
			("Controller::State: Unable to initialize at least *one* device");
	}

	Path p;
	std::string fullConfigPath = p.Expand(configFilename);

	VIP::XmlInitializer initializer(this, actions);

	std::vector<VipPtrT> allVips;
	initializer.traverse(fullConfigPath, allVips);

	BOOST_FOREACH(VipPtrT vip, allVips)
	{
		debugOut(vip, actions);

		if (vip->mDevice == ID::DT_Keyboard)
			assignToDevice(vip, mKeyboard);
		else if (vip->mDevice == ID::DT_Mouse)
			assignToDevice(vip, mMouse);
		else if (vip->mDevice == ID::DT_Joystick)
			assignToDevice(vip, mJoystick);
		
		visitFeedback(vip);
	}
}

void State::capture()
{
	if (mMouse.good())
		mMouse.capture();
	
	if (mKeyboard.good())
		mKeyboard.capture();

	if (mJoystick.good())
		mJoystick.capture();
}

void State::sendFeedback(long microseconds_passed)
{
	for_each
	(
	    mFeedbacks.begin(),
	    mFeedbacks.end(),
	    boost::bind(&VIP::CommonBase::FeedTime, _1, microseconds_passed)
	);
}

EventLoop* State::eventLoop() const
{
	return mEventLoop;
}

const std::string& State::getStateID() const
{
	return mID;
}

void State::activate()
{
	// This will fix a bug which can occur on state change.
	disableFeedbackOfAllVips();

	if (mMouse.good())
		mMouse.activate();

	if (mKeyboard.good())
		mKeyboard.activate();

	if (mJoystick.good())
		mJoystick.activate();
}

void State::deactivate()
{
	if (mMouse.good())
		mMouse.deactivate();

	if (mKeyboard.good())
		mKeyboard.deactivate();

	if (mJoystick.good())
		mJoystick.deactivate();
}

void State::visitFeedback(VipPtrT Vip)
{
	mFeedbacks.insert(Vip);
}

void State::disableFeedbackOfAllVips()
{
	FeedbackT::iterator it = mFeedbacks.begin();
	for (; it != mFeedbacks.end(); ++it)
		(*it)->DisableFeedback();
}

} // namespace Controller_
} // namespace BFG
