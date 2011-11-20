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

#ifndef BFG_PHYSICS_OBJECT_H__
#define BFG_PHYSICS_OBJECT_H__

#include <boost/shared_ptr.hpp>
#include <boost/units/systems/si/mass.hpp>
#include <boost/units/systems/si/time.hpp>
#include <boost/units/quantity.hpp>

#include <ode/ode.h>

#include <map>

#include <Core/Types.h>
#include <Core/v3.h>
#include <Core/qv4.h>
#include <Core/ExternalTypes_fwd.h>

#include <EventSystem/Emitter.h>

#include <Physics/Defs.h>
#include <Physics/Event_fwd.h>
#include <Model/Enums.hh>

using namespace boost::units;

namespace BFG {
namespace Physics {

class OdeTriMesh;

struct PHYSICS_API GeomData
{
	dGeomID           geomId;
	f32               density;
	ID::CollisionMode collisionMode;
};

class PHYSICS_API PhysicsObject : Emitter
{
public:
	PhysicsObject(EventLoop* loop,
	              dWorldID worldId,
	              dSpaceID spaceId,
	              const Location& location);
	              
	~PhysicsObject();

	void addModule(GameHandle moduleHandle,
	               const std::string& meshName,
				   ID::CollisionMode collisionMode,
				   const v3& position,
				   const qv4& orientation,
	               f32 density);

	void addModule(boost::shared_ptr<PhysicsObject>,
	               const v3& position,
	               const qv4& orientation);

	void removeModule(boost::shared_ptr<PhysicsObject> po,
	                  const v3& position,
	                  const qv4& orientation);

	void prepareOdeStep(quantity<si::time, f32> stepsize);
	
	void clearForces();
	
	ID::CollisionMode getCollisionMode(GameHandle moduleHandle) const;

	bool hasModule(GameHandle moduleHandle) const;

	void sendFullSync() const;

private:
	void debugOutput(std::string& output) const;

	void setPosition(const v3& pos);
	void setOrientation(const qv4& rot);

	void setOffsetPosition(GameHandle moduleHandle, const v3& pos);
	void setOffsetOrientation(GameHandle moduleHandle, const qv4& rot);

	v3 getPosition() const;
	qv4 getOrientation() const;

	v3 getVelocity() const;
	v3 getVelocityRelative() const;
	void setVelocity(const v3& velocity);

	v3 getRotationVelocity() const;
	v3 getRotationVelocityRelative() const;
	void setRotationVelocity(const v3& rotVelocity);

	//! Sets ID::CollisionMode for the whole object. Note that all
	//! individual ID::CollisionMode settings (per Module/Geom) will get lost.
	void setCollisionMode(ID::CollisionMode cm);

	void enableSimulation(bool enable);

	void recalculateMass();

	void updateForces(const v3& force, const v3& torque);

	void magicStop();
	void modulateMass(f32 factor) const;
	
	quantity<si::mass, f32> getTotalWeight() const;
	void                    getTotalInertia(m3x3& inertia) const;

	typedef std::map<std::string, boost::shared_ptr<OdeTriMesh> > MeshCacheT;

	void registerEvents();
	void unregisterEvents();

	void eventHandler(Physics::Event*);

	void notifyControlAboutChangeInMass() const;

	void setCollisionModeGeom(GameHandle moduleHandle,
	                          ID::CollisionMode cm);

	void onForce(const v3& force);
	void onTorque(const v3& torque);

	typedef std::map
	<
		GameHandle,
		GeomData
	> GeomMapT;

	dBodyID           mOdeBody;
	GeomMapT          mGeometry;
	dSpaceID          mSpaceId;

	GameHandle        mRootModule;
	v3                mBodyOffset;
	
	dMass             mOriginalMass;  

	v3                mForce;
	v3                mTorque;
	
	ID::CollisionMode mCollisionMode;
	
	std::vector<ID::PhysicsAction> mPhysicsEvents;
	
	static MeshCacheT mMeshCache;
	
	friend std::ostream& operator << (std::ostream& lhs, const PhysicsObject& rhs);
};

} // namespace Physics
} // namespace BFG

#endif
