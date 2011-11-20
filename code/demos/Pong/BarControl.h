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

#ifndef __PONG_BAR_CONTROL_H_
#define __PONG_BAR_CONTROL_H_

#include <Core/Location.h>
#include <Core/Math.h>
#include <Physics/PhysicsManager.h>
#include <Model/GameObject.h>
#include <Model/Property/Concept.h>

#include "Pong/PongDefinitions.h"

class BarControl : public BFG::Property::Concept
{
public:
	BarControl(BFG::GameObject& Owner, BFG::PluginId pid) :
	BFG::Property::Concept(Owner, "BarControl", pid)
	{
		require("Physical");
		requestEvent(BFG::ID::GOE_CONTROL_YAW);
	}
	
	void internalUpdate(quantity<si::time, BFG::f32> timeSinceLastFrame)
	{
		using namespace BFG;

		Location go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);

		// Simulate a wall
		if (std::abs(go.position.x) > DISTANCE_TO_WALL)
		{
			emit<Physics::Event>(ID::PE_UPDATE_VELOCITY, v3::ZERO, ownerHandle());
			go.position.x = sign(go.position.x) * (DISTANCE_TO_WALL - 0.01f);
		}
	
		// Make sure it doesn't move to much on the z axis
		if (std::abs(go.position.z) - OBJECT_Z_POSITION > EPSILON_F)
		{
			go.position.z = OBJECT_Z_POSITION + SPECIAL_PACKER_MESH_OFFSET;
		}

		emit<Physics::Event>(ID::PE_UPDATE_POSITION, go.position, ownerHandle());
		emit<Physics::Event>(ID::PE_UPDATE_ORIENTATION, qv4::IDENTITY, ownerHandle());
	}
	
	void internalOnEvent(EventIdT action,
	                     BFG::Property::Value payload,
	                     BFG::GameHandle module,
	                     BFG::GameHandle sender)
	{
		using namespace BFG;
	
		switch(action)
		{
			case ID::GOE_CONTROL_YAW:
			{
				const f32 barSpeedMultiplier = 45.0f;
				emit<Physics::Event>
				(
					ID::PE_UPDATE_VELOCITY,
					v3(payload * barSpeedMultiplier, 0, 0),
					ownerHandle()
				);
				break;
			}
		}
	}
};

#endif //__PONG_BAR_CONTROL_H_
