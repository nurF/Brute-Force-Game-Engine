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

#include <Model/Property/Concepts/Physical.h>

#include <boost/foreach.hpp>

#include <Core/Math.h>
#include <Model/Environment.h>
#include <Physics/Event.h>
#include <View/Event.h>

namespace BFG {

Physical::Physical(GameObject& owner, PluginId pid) :
Property::Concept(owner, "Physical", pid)
{
	mPhysicsActions.push_back(ID::PE_FULL_SYNC);
	mPhysicsActions.push_back(ID::PE_POSITION);
	mPhysicsActions.push_back(ID::PE_ORIENTATION);
	mPhysicsActions.push_back(ID::PE_VELOCITY);
	mPhysicsActions.push_back(ID::PE_ROTATION_VELOCITY);
	mPhysicsActions.push_back(ID::PE_TOTAL_MASS);
	mPhysicsActions.push_back(ID::PE_TOTAL_INERTIA);

	BOOST_FOREACH(ID::PhysicsAction action, mPhysicsActions)
	{
		loop()->connect(action, this, &Physical::onPhysicsEvent, ownerHandle());
	}
}

Physical::~Physical()
{
	BOOST_FOREACH(ID::PhysicsAction action, mPhysicsActions)
	{
		loop()->disconnect(action, this);
	}

}

void Physical::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	assert(ownerHandle() != NULL_HANDLE);
	
	synchronizeView();
}

void Physical::onPhysicsEvent(Physics::Event* event)
{
	switch(event->getId())
	{
	case ID::PE_FULL_SYNC:
		onFullSync(boost::get<Physics::FullSyncData>(event->getData()));
		break;

	case ID::PE_VELOCITY:
		onVelocity(boost::get<Physics::VelocityComposite>(event->getData()));
		break;

	case ID::PE_ROTATION_VELOCITY:
		onRotationVelocity(boost::get<Physics::VelocityComposite>(event->getData()));
		break;

	case ID::PE_POSITION:
		onPosition(boost::get<v3>(event->getData()));
		break;

	case ID::PE_ORIENTATION:
		onOrientation(boost::get<qv4>(event->getData()));
		break;

	case ID::PE_TOTAL_MASS:
		onTotalMass(boost::get<f32>(event->getData()));
		break;

	case ID::PE_TOTAL_INERTIA:
		onInertia(boost::get<m3x3>(event->getData()));
		break;

	default:
	{
		std::stringstream ss;
		ss << "void Physical::onPhysicsEvent(Physics::Event*): "
			"Got unknown event";
		throw std::runtime_error(ss.str());
	}
	}
}

void Physical::onFullSync(const Physics::FullSyncData& fsd)
{
	Location location(fsd.get<0>(), fsd.get<1>());
	setGoValue(ID::PV_Location, pluginId(), location);

	setGoValue(ID::PV_Velocity, pluginId(), fsd.get<2>());
	setGoValue(ID::PV_RelativeVelocity, pluginId(), fsd.get<3>());

	setGoValue(ID::PV_RotationVelocity, pluginId(), fsd.get<4>());
	setGoValue(ID::PV_RelativeRotationVelocity, pluginId(), fsd.get<5>());
	
	setGoValue(ID::PV_Mass, pluginId(), fsd.get<6>());
	setGoValue(ID::PV_Inertia, pluginId(), fsd.get<7>());
	
	owner().activate();
}

void Physical::onPosition(const v3& newPosition)
{
	Location go = getGoValue<Location>(ID::PV_Location, pluginId());
	go.position = newPosition;
	setGoValue(ID::PV_Location, pluginId(), go);
}

void Physical::onOrientation(const qv4& newOrientation)
{
	Location go = getGoValue<Location>(ID::PV_Location, pluginId());
	go.orientation = newOrientation;
	setGoValue(ID::PV_Location, pluginId(), go);
}

void Physical::onVelocity(const Physics::VelocityComposite& vel)
{
	setGoValue(ID::PV_Velocity, pluginId(), vel.get<0>());
	setGoValue(ID::PV_RelativeVelocity, pluginId(), vel.get<1>());
}

void Physical::onRotationVelocity(const Physics::VelocityComposite& vel)
{
	setGoValue(ID::PV_RotationVelocity, pluginId(), vel.get<0>());
	setGoValue(ID::PV_RelativeRotationVelocity, pluginId(), vel.get<1>());
}

void Physical::onTotalMass(const f32 totalMass)
{
	setGoValue(ID::PV_Mass, pluginId(), totalMass);
}

void Physical::onInertia(const m3x3& inertia)
{
	setGoValue(ID::PV_Inertia, pluginId(), inertia);
}

void Physical::synchronizeView() const
{
	if (owner().docked())
		return;

	const Location& go = getGoValue<Location>(ID::PV_Location, pluginId());
	
#if defined(_DEBUG) || defined (NDEBUG)
	if (length(go.position) > 1.0e15)
	{
		std::stringstream ss;
		ss << "GameObject:" << ownerHandle()
		   << " just entered hyperspace! Position: "
		   << go.position;

		throw std::logic_error(ss.str());
	}
#endif

	emit<View::Event>(ID::VE_UPDATE_POSITION, go.position, ownerHandle());
	emit<View::Event>(ID::VE_UPDATE_ORIENTATION, go.orientation, ownerHandle());

	//! \todo Gui still requires these events
	emit<GameObjectEvent>(ID::GOE_POSITION, go.position, 0, ownerHandle());
	emit<GameObjectEvent>(ID::GOE_ORIENTATION, go.orientation, 0, ownerHandle());
}

} // namespace BFG