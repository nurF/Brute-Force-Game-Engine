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

#ifndef __PONG_STATE_H_
#define __PONG_STATE_H_

#include <EventSystem/Emitter.h>
#include <EventSystem/EventFactory.h>

#include <Core/ClockUtils.h>
#include <Core/Path.h>
#include <Core/Types.h>
#include <Core/Utils.h>

#include <Controller/ControllerEvents.h>
#include <Controller/ControllerInterface.h>

#include <Model/Loader/Types.h>
#include <Model/Loader/Interpreter.h>
#include <Model/Loader/GameObjectFactory.h>

#include <Model/Environment.h>
#include <Model/Property/SpacePlugin.h>

#include <Physics/PhysicsManager.h>

#include "Pong/PongDefinitions.h"
#include "Pong/PongFactory.h"
#include <Pong/PropertyPlugin.h>

struct PongServerState : BFG::Emitter
{
	PongServerState(BFG::GameHandle handle, EventLoop* loop) :
	Emitter(loop),
	mClock(new BFG::Clock::StopWatch(BFG::Clock::milliSecond)),
	mExitNextTick(false)
	{
		BFG::Path p;
		std::string level = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "pong/";
		std::string def = p.Get(BFG::ID::P_SCRIPTS_LEVELS) + "default/";

		BFG::Loader::LevelConfig lc;

		lc.mModules.push_back(def + "Object.xml");
		lc.mAdapters.push_back(def + "Adapter.xml");
		lc.mConcepts.push_back(def + "Concept.xml");
		lc.mProperties.push_back(def + "Value.xml");

		lc.mModules.push_back(level + "Object.xml");
		lc.mAdapters.push_back(level + "Adapter.xml");
		lc.mConcepts.push_back(level + "Concept.xml");
		lc.mProperties.push_back(level + "Value.xml");

		boost::shared_ptr<BFG::Environment> environment(new BFG::Environment);

		using BFG::Property::ValueId;
		using BFG::Property::PluginId;

		PluginId spId = ValueId::ENGINE_PLUGIN_ID;
		PluginId ppId = BFG::Property::generatePluginId<PluginId>();

		boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
		boost::shared_ptr<PongPlugin> pp(new PongPlugin(ppId));

		pm.insert(sp);
		pm.insert(pp);

		boost::shared_ptr<BFG::Loader::Interpreter> interpreter(new BFG::Loader::Interpreter(pm));

		boost::shared_ptr<BFG::Loader::GameObjectFactory> gof;
		gof.reset(new BFG::Loader::GameObjectFactory(loop, lc, pm, interpreter, environment, handle));

		// Hack: We don't use the SectorFactory directly, since there's still
		//       race-related stuff inside
		BFG::Loader::ObjectParameter op;
		op.mHandle = BFG::generateHandle();
		op.mName = "Ball";
		op.mType = "Pong Ball";
		op.mLocation = v3(0.0f, 0.0f, OBJECT_Z_POSITION);
		op.mLinearVelocity = BALL_START_VELOCITY;
		mBall = gof->createGameObject(op);

		op = BFG::Loader::ObjectParameter();
		op.mHandle = 123;
		op.mName = "Lower Bar";
		op.mType = "Pong Lower Bar";
		op.mLocation = v3(0.0f, -BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);
		mLowerBar = gof->createGameObject(op);

		op = BFG::Loader::ObjectParameter();
		op.mHandle = 456;
		op.mName = "Upper Bar";
		op.mType = "Pong Upper Bar";
		op.mLocation.position = v3(0.0f, BAR_Y_POSITION, OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET);
		op.mLocation.orientation = BFG::qv4::IDENTITY;
		fromAngleAxis(op.mLocation.orientation, 180 * DEG2RAD, BFG::v3::UNIT_Z);

		mUpperBar = gof->createGameObject(op);

		mClock->start();
	}

	void LoopEventHandler(LoopEvent* iLE)
	{
		if (mExitNextTick)
		{
			// Error happened, while doing stuff
			iLE->getData().getLoop()->setExitFlag();
		}

		long timeSinceLastFrame = mClock->stop();
		if (timeSinceLastFrame)
			mClock->start();

		BFG::f32 timeInSeconds = static_cast<BFG::f32>(timeSinceLastFrame) / BFG::Clock::milliSecond;
		tick(timeInSeconds);
	}

	void tick(const BFG::f32 timeSinceLastFrame)
	{
		if (timeSinceLastFrame < BFG::EPSILON_F)
			return;

		quantity<si::time, BFG::f32> TSLF = timeSinceLastFrame * si::seconds;

		mBall->update(TSLF);
		mLowerBar->update(TSLF);
		mUpperBar->update(TSLF);

		emit<BFG::Physics::Event>(BFG::ID::PE_STEP, TSLF.value());
	}

	boost::shared_ptr<BFG::GameObject> mBall;
	boost::shared_ptr<BFG::GameObject> mLowerBar;
	boost::shared_ptr<BFG::GameObject> mUpperBar;

	boost::scoped_ptr<BFG::Clock::StopWatch> mClock;

	BFG::Property::PluginMapT pm;

	bool mExitNextTick;
};

struct PongClientState : BFG::Emitter
{
	PongClientState(BFG::GameHandle handle, EventLoop* loop) :
	Emitter(loop),
	mExitNextTick(false),
	mPlayer(123)
	{

	}

	void ControllerEventHandler(BFG::Controller_::VipEvent* iCE)
	{
		switch(iCE->getId())
		{
		// Using this as lower bar x-axis controller (Sending to: Lower Bar!)
		case A_SHIP_AXIS_Y:
			if (!mPlayer)
				return;

			emit<BFG::GameObjectEvent>(BFG::ID::GOE_CONTROL_YAW, boost::get<float>(iCE->getData()), mPlayer);
			break;

		case A_QUIT:
		{
			mExitNextTick = true;
			break;
		}

		case A_FPS:
		{
			emit<BFG::View::Event>(BFG::ID::VE_DEBUG_FPS, boost::get<bool>(iCE->getData()));
			break;
		}
		}
	}

	void LoopEventHandler(LoopEvent* iLE)
	{
		if (mExitNextTick)
		{
			// Error happened, while doing stuff
			iLE->getData().getLoop()->setExitFlag();
		}
	}

	BFG::GameHandle mPlayer;
	
	bool mExitNextTick;
};

#endif //__PONG_STATE_H_
