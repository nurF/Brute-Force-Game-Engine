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

#include <Model/Property/Concepts/Destroyable.h>

#include <boost/units/systems/si/mass.hpp>

#include <Base/CLogger.h>
#include <Core/Math.h>

#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Module.h>

#include <Model/Events/GameObjectEvent.h>
#include <Model/Events/SectorEvent.h>
#include <View/Event.h>
#include <Physics/Event.h>

namespace BFG {

Destroyable::Destroyable(GameObject& owner, PluginId pid) :
Property::Concept(owner, "Destroyable", pid),
mDamageMultiplier(100.0f)
{
	initvar(ID::PV_Destroyed);
	initvar(ID::PV_Respawns);
	initvar(ID::PV_Armor);
	initvar(ID::PV_Damage);
	initvar(ID::PV_RespawnCountdown);
	initvar(ID::PV_Effect);

	requestEvent(ID::PE_CONTACT);
	requestEvent(ID::GOE_REINITIALIZE);
	
	require("Physical");
}

void Destroyable::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	ModuleMapT::iterator it = mModules.begin();
	for (; it != mModules.end(); ++it)
		updateModule(it->first, timeSinceLastFrame);
}

void Destroyable::internalOnEvent(EventIdT action,
                                  Property::Value payload,
                                  GameHandle module,
                                  GameHandle sender)
{
	if (mModules.find(module) == mModules.end())
		return;

	switch(action)
	{
	case ID::PE_CONTACT:
	{
		f32 newDamage = payload * mDamageMultiplier;
		value<f32>(ID::PV_Damage, module) += newDamage;
		break;
	}
	
	case ID::GOE_REINITIALIZE:
	{
		value<f32>(ID::PV_Damage, module)     = 0.0f;
		value<bool>(ID::PV_Destroyed, module) = false;
		break;
	}
	
	default:
		throw std::logic_error
			("Destroyable::internalOnEvent: Got unknown event!");
	}
}

void Destroyable::updateModule(GameHandle module,
                               quantity<si::time, f32> timeSinceLastFrame)
{
	bool destroyed = value<bool>(ID::PV_Destroyed, module);
	bool respawns  = value<bool>(ID::PV_Respawns, module);
	
	f32 armor      = value<f32>(ID::PV_Armor, module);
	f32 damage     = value<f32>(ID::PV_Damage, module);

	updateGui(damage, armor);

	if (destroyed && respawns)
	{
		if (shouldRespawn(module, timeSinceLastFrame))
		{
			respawn(module);
		}
	}
	// If not yet destroyed, but too damaged
	else if (! destroyed && damage > armor)
	{
		destroy(module, respawns);
	}
}

void Destroyable::updateGui(f32 damage, f32 armor)
{
	f32 remainingArmor = (1.0f - damage / armor) * 100;
	s32 value = clamp((s32)remainingArmor, 0, 100);
	emit<GameObjectEvent>(ID::GOE_ARMOR, value, 0, ownerHandle());
}

bool Destroyable::isHeavy(GameHandle handle) const
{
	//! \note
	//! Using weight of the whole object for now (which is not correct - it
	//! should be the weight of the individual module)
	
	typedef quantity<si::mass, f32> MassT;
	const MassT& weight = getGoValue<f32>(ID::PV_Mass, pluginId()) * si::kilogram;
	
	const MassT MinimumWeightForBigExplosion = 2000.0f * si::kilogram;
	
	return weight > MinimumWeightForBigExplosion;
}

void Destroyable::destroy(GameHandle module, bool respawns)
{
	value<bool>(ID::PV_Destroyed, module) = true;
	mTimeSinceDestruction[module] = 0.0f;

	const Location& go = getGoValue<Location>(ID::PV_Location, pluginId());

	CharArray128T effect = value<CharArray128T>(ID::PV_Effect, module);

	//! \todo calculate intensity
	f32 intensity = 1.0f + (f32)isHeavy(module);
	
	View::EffectCreation ec(effect, go.position, intensity);
	warnlog << "Destroyable: VE_EFFECT event may arrive at undesired locations due to unknown view state handle";
	emit<View::Event>(ID::VE_EFFECT, ec);

	if (respawns)
	{
		emit<Physics::Event>(ID::PE_UPDATE_COLLISION_MODE, (s32) ID::CM_Disabled, module);
		emit<Physics::Event>(ID::PE_UPDATE_SIMULATION_FLAG, false, module);
		emit<View::Event>(ID::VE_SET_VISIBLE, false, module);
	}
	else
	{
		emit<SectorEvent>(ID::S_DESTROY_GO, ownerHandle());
	}
}

void Destroyable::respawn(GameHandle module)
{
	dbglog << "Respawning Module ID: " << module;

	// Spawn "behind" (negative Z axis) the collision point
	const f32 distance_in_meters = 50.0f;

	const Location& go = getGoValue<Location>(ID::PV_Location, pluginId());

	Location nextSpawn = value<Location>(ID::PV_Location, module);
	nextSpawn.position += go.position;
	nextSpawn.position -= go.orientation.zAxis() * distance_in_meters;
	nextSpawn.orientation = go.orientation;

	emit<GameObjectEvent>(ID::GOE_REINITIALIZE, nextSpawn, ownerHandle());

	emit<Physics::Event>(ID::PE_UPDATE_COLLISION_MODE, (s32) ID::CM_Standard, module);
	emit<Physics::Event>(ID::PE_UPDATE_SIMULATION_FLAG, true, module);

	emit<View::Event>(ID::VE_SET_VISIBLE, true, module);
	
	mTimeSinceDestruction[module]         = 0.0f;
	value<bool>(ID::PV_Destroyed, module) = false;
}

bool Destroyable::shouldRespawn(GameHandle module,
                                quantity<si::time, f32> timeSinceLastFrame)
{
	mTimeSinceDestruction[module] += timeSinceLastFrame.value();

	// Check if Countdown has been reached
	f32 CD = value<f32>(ID::PV_RespawnCountdown, module);
	
	// If so, return true
	return mTimeSinceDestruction[module] > CD;
}

} // namespace BFG
