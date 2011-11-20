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

#ifndef PHYSICS_EVENT_FWD_H
#define PHYSICS_EVENT_FWD_H

#include <boost/variant.hpp>
#include <boost/units/quantity.hpp>
#include <boost/units/systems/si/mass.hpp>

#include <Core/ExternalTypes.h>
#include <Core/Location.h>
#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Event_fwd.h>
#include <Physics/Enums.hh>

namespace BFG {
namespace Physics {

using namespace boost::units;

typedef boost::tuple
<
	v3,                       // Position
	qv4,                      // Orientation
	v3,                       // Velocity
	v3,                       // Relative Velocity
	v3,                       // Rotation Velocity
	v3,                       // Relative Rotation Velocity
	f32,                      // Total Mass
	m3x3                      // Inertia
> FullSyncData;

typedef boost::tuple
<
	v3,                  // Absolute
	v3                   // Relative
> VelocityComposite;

typedef boost::tuple
<
	GameHandle,
	Location
> ObjectCreationParams;

typedef boost::tuple
<
	GameHandle,          // Handle of PhysicsObject
	GameHandle,          // Module Handle
	std::string,         // Mesh Name (e.g. "Cube.mesh")
	ID::CollisionMode,
	v3,                  // Module Position
	qv4,                 // Module Orientation
	f32                  // Density
> ModuleCreationParams;

typedef boost::tuple
<
	GameHandle,          // Handle of PhysicsObject
	GameHandle           // Module Handle
> ModuleRemovalParams;

typedef boost::tuple
<
	GameHandle,          // Parent
	GameHandle,          // Child
	v3,                  // Offset Position
	qv4                  // Offset Orientation
> ObjectAttachmentParams;

typedef boost::variant
<
	bool,
	s32,
	f32,
	v3,
	qv4,
	m3x3,
	GameHandle,
	FullSyncData,
	VelocityComposite,
	ObjectCreationParams,
	ModuleCreationParams,
	ModuleRemovalParams,
	ObjectAttachmentParams
> PayloadT;

typedef Event
<
	EventIdT,
	PayloadT,
	GameHandle,
	GameHandle
>
Event;
} // namespace Physics
} // namespace BFG

#endif