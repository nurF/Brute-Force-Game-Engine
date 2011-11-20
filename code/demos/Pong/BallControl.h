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

#ifndef __PONG_BALL_CONTROL_H_
#define __PONG_BALL_CONTROL_H_

#include <Core/Math.h>
#include <Physics/PhysicsManager.h>
#include <Model/GameObject.h>
#include <Model/Property/Concept.h>
#include <View/Event.h>

#include "Pong/PongDefinitions.h"

class BallControl : public BFG::Property::Concept
{
public:
	BallControl(BFG::GameObject& Owner, BFG::PluginId pid) :
	BFG::Property::Concept(Owner, "BallControl", pid)
	{
		require("Physical");
	}
	
	void internalUpdate(quantity<si::time, BFG::f32> timeSinceLastFrame)
	{
		using namespace BFG;
	
		Location go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);
		v3 newVelocity = getGoValue<v3>(ID::PV_Velocity, ValueId::ENGINE_PLUGIN_ID);

		// Simulate a wall
		if (std::abs(go.position.x) > DISTANCE_TO_WALL)
		{
			newVelocity.x = std::abs(newVelocity.x) * -BFG::sign(go.position.x);
		}

		// Make sure it doesn't move on the z axis
		if (std::abs(go.position.z - OBJECT_Z_POSITION) > BFG::EPSILON_F)
		{
			go.position.z = OBJECT_Z_POSITION;
			newVelocity.z = 0.0;
		}
		
		// If it passed through a player bar, set it back to zero
		const f32 ball_reset_distance = 15.0f;
		
		bool lossDetected = false;

		// Upper Bar Loss?
		if (go.position.y > ball_reset_distance)
		{
			emit<BFG::View::Event>(static_cast<ID::ViewAction>(A_LOWER_BAR_WIN), 1);
			lossDetected = true;
		}
		// Lower Bar Loss?
		else if (go.position.y < -ball_reset_distance)
		{
			emit<BFG::View::Event>(static_cast<ID::ViewAction>(A_UPPER_BAR_WIN), 1);
			lossDetected = true;
		}

		if (lossDetected)
		{
			go.position = v3(0.0f, 0.0f, OBJECT_Z_POSITION);
			newVelocity   = BALL_START_VELOCITY;
		}

		f32 currentSpeed = BFG::length(newVelocity);
		f32 startSpeed = BFG::length(BALL_START_VELOCITY);
		
		if (currentSpeed < startSpeed)
		{
			newVelocity *= startSpeed / currentSpeed;
		}

		if (std::abs(newVelocity.y) < DESIRED_SPEED)
		{
			newVelocity.y *= std::abs(DESIRED_SPEED / newVelocity.y);
		}

		emit<Physics::Event>(ID::PE_UPDATE_POSITION, go.position, ownerHandle());
		emit<Physics::Event>(ID::PE_UPDATE_VELOCITY, newVelocity, ownerHandle());
	}
};

#endif //__PONG_BALL_CONTROL_H_
