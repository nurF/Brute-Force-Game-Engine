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

#ifndef __PHYSICS_MANAGER__
#define __PHYSICS_MANAGER__

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/units/systems/si/time.hpp>
#include <boost/units/quantity.hpp>

#include <map>

#include <ode/common.h>
#include <utility>

#include <EventSystem/Emitter.h>

#include <Core/Types.h>
#include <Core/v3.h>
#include <Core/qv4.h>

#include <Physics/Defs.h>
#include <Physics/Event_fwd.h>

using namespace boost::units;

namespace BFG {
namespace Physics {

class PhysicsObject;

/**
	float: radius of sphere
	v3: side lengths
	v4: plane data: ax + by + cz = d
	std::pair<f32, f32> radius, length of Cylinder
	std::string: mesh name
*/
typedef boost::variant <
	f32,
	v3,
	qv4,
	std::pair<f32, f32>,
	std::string
> CollisionTypeData;

class PHYSICS_API PhysicsManager : Emitter
{

public:
	PhysicsManager(EventLoop* loop, u32 maxContactsPerCollision = 30);
	~PhysicsManager();

	//! return the ODE HashSpaceID
	dSpaceID getSpaceID() const;

	//! return the ODE world
	dWorldID getWorldID() const;

	void clear();

	void move(quantity<si::time, f32> timeSinceLastFrame);

	void addObject(GameHandle handle,
	               boost::shared_ptr<PhysicsObject> object);

#if 0
	void createPhysicalObject(GameHandle handle,
		                      ID::CollisionGeometry geomType, 
	                          CollisionTypeData dat, 
	                          ID::CollisionType colType,
	                          bool collidable,
	                          bool simulated);
#endif	

private:
	typedef std::map
	<
		GameHandle,
		boost::shared_ptr<PhysicsObject>
	> PhysicsObjectMap;

	boost::shared_ptr<PhysicsObject> searchMovObj(GameHandle handle) const;

	void onNearCollision(dGeomID geo1, dGeomID geo2);
	void collideGeoms(dGeomID geo1, dGeomID geo2) const;

	void registerEvents();
	void unregisterEvents();
	void eventHandler(Physics::Event*);

	void onCreateObject(const ObjectCreationParams& ocp);
	void onDeleteObject(GameHandle);
	void onAttachModule(const ModuleCreationParams&);
	void onRemoveModule(const ModuleRemovalParams&);

	const u32                     mMaxContactsPerCollision;
	const quantity<si::time, f32> mSimulationStepSize;

	dWorldID mWorldID;
	dSpaceID mHashSpaceID;

	//! ODE Contact Joints (if needed)
	dJointGroupID mCollisionJointsID;
	
	PhysicsObjectMap mPhysicsObjects;

	std::vector<ID::PhysicsAction> mPhysicsEvents;

	friend void globalOdeNearCollisionCallback(void* additionalData,
	                                           dGeomID geo1,
	                                           dGeomID geo2);
};

} // namespace Physics
} // namespace BFG

#endif
