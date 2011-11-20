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

#include <OgreException.h>

#include <Base/CEntryPoint.h>
#include <Base/CLogger.h>
#include <Base/Network.h>
#include <Base/Pause.h>
#include <Controller/Action.h>
#include <EventSystem/Core/EventManager.h>
#include <Core/ShowException.h>
#include <Model/Interface.h>
#include <Physics/Interface.h>

#include "Pong/PongDefinitions.h"
#include "Pong/BallControl.h"
#include "Pong/BarControl.h"
#include "Pong/PongFactory.h"
#include "Pong/PongState.h"
#include "Pong/ViewPongState.h"

BFG::GameHandle stateHandle = 12345;

// This is the Ex-'GameStateManager::SingleThreadEntryPoint(void*)' function
void* ClientEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	
	GameHandle pongClientHandle = stateHandle;
	
	// Hack: Using leaking pointers, because vars would go out of scope
	PongClientState* ps = new PongClientState(pongClientHandle, loop);
	ViewPongState* vps = new ViewPongState(pongClientHandle, loop);

	// Init Controller
	BFG::GameHandle handle = BFG::generateHandle();

	{
		BFG::Controller_::ActionMapT actions;
		actions[A_SHIP_AXIS_Y] = "A_SHIP_AXIS_Y";
		actions[A_QUIT]        = "A_QUIT";
		actions[A_FPS]         = "A_FPS";
		BFG::Controller_::sendActionsToController(loop, actions);
	
		BFG::Path path;
		const std::string config_path = path.Expand("Pong.lua");
		const std::string state_name = "Pong";
		
		BFG::Controller_::StateInsertion si(config_path, state_name, handle, true);

		BFG::EventFactory::Create<BFG::Controller_::ControlEvent>
		(
			loop,
			BFG::ID::CE_LOAD_STATE,
			si
		);

		loop->connect(A_SHIP_AXIS_Y, ps, &PongClientState::ControllerEventHandler);
		loop->connect(A_QUIT, ps, &PongClientState::ControllerEventHandler);
		loop->connect(A_FPS, ps, &PongClientState::ControllerEventHandler);
	}

	assert(loop);
	loop->registerLoopEventListener(ps, &PongClientState::LoopEventHandler);
	return 0;
}

void* ServerEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);

	GameHandle pongServerHandle = stateHandle;

	// Hack: Using leaking pointers, because vars would go out of scope
	PongServerState* ps = new PongServerState(pongServerHandle, loop);

	assert(loop);
	loop->registerLoopEventListener(ps, &PongServerState::LoopEventHandler);
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	BFG::Base::Logger::Init(BFG::Base::Logger::SL_DEBUG, "Logs/Pong.log");

	infolog << "Hello World of Pong!";

	bool server = false;
	if  (argc > 1)
		server = true;
			
	if (server)
	{
		EventManager::getInstance()->listen(1337);

		EventLoop loop1
		(
			true,
			new EventSystem::BoostThread<>("Loop1"),
			new EventSystem::InterThreadCommunication()
		);

		using namespace boost::posix_time;
		boost::this_thread::sleep(seconds(10));

 		loop1.addEntryPoint(BFG::ModelInterface::getEntryPoint());
 		loop1.addEntryPoint(BFG::Physics::Interface::getEntryPoint());
		loop1.addEntryPoint(new BFG::Base::CEntryPoint(ServerEntryPoint));
		loop1.run();

		//boost::this_thread::sleep(seconds(1));

		//EventLoop loop2
		//(
		//	true,
		//	new EventSystem::BoostThread<>(),
		//	new EventSystem::InterThreadCommunication()
		//);

		//loop2.run();

		BFG::Base::pause();
	}
	else
	{
		std::string ip;
		BFG::Base::resolveDns("79.215.142.134", ip);
		EventManager::getInstance()->connect(ip, 1337);

		EventLoop iLoop
		(
			true,
			new EventSystem::BoostThread<>("Loop1"),
			new EventSystem::InterThreadCommunication()
		);

		size_t controllerFrequency = 1000;

		iLoop.addEntryPoint(BFG::View::Interface::getEntryPoint("Engine Test 01: Pong"));
		iLoop.addEntryPoint(BFG::ControllerInterface::getEntryPoint(controllerFrequency));
		iLoop.addEntryPoint(new BFG::Base::CEntryPoint(ClientEntryPoint));
		iLoop.run();
		
		BFG::Base::pause();
	}

	infolog << "Good Bye!";
}
catch (Ogre::Exception& e)
{
	BFG::showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	BFG::showException(ex.what());
}
catch (...)
{
	BFG::showException("Unknown exception");
}

