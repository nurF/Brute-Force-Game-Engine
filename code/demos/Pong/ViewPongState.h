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

#ifndef __VIEW_PONG_STATE_H_
#define __VIEW_PONG_STATE_H_

#include <Core/Utils.h>

#include <View/Owner.h>
#include <View/Interface.h>
#include <View/Event.h>
#include <View/State.h>
#include <View/RenderObject.h>
#include <View/Main.h>

#include <Pong/Score.h>

struct ViewPongState : public BFG::View::State
{
public:
	ViewPongState(BFG::GameHandle handle, EventLoop* loop) :
	State(handle, loop)
	{
		registerEventHandler();
	}

	~ViewPongState()
	{
		unregisterEventHandler();
	}

	void viewEventHandler(BFG::View::Event* VE)
	{
	}


	bool frameStarted(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	bool frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	bool frameEnded(const Ogre::FrameEvent& evt)
	{
		return true;
	}

private:
	void registerEventHandler(){}
	void unregisterEventHandler(){}

	virtual void pause(){ registerEventHandler(); }
	virtual void resume(){ unregisterEventHandler(); }

	BFG::View::Score mScore;
};

#endif //__VIEW_PONG_STATE_H_
