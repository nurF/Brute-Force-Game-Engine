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

#include <Physics/PhysicsObject.h>

#include <boost/foreach.hpp>

#include <ode/../../ode/src/objects.h>   // for debugOutput()
#include <ode/../../ode/src/collision_kernel.h>   // for debugOutput()

#include <Base/CLogger.h>

#include <Core/ExternalTypes.h>
#include <Core/Math.h>

#include <View/LoadMesh.h>

#include <Model/Events/GameObjectEvent.h>

#include <Physics/Event.h>
#include <Physics/OdeTriMesh.h>


namespace BFG {
namespace Physics {

PhysicsObject::MeshCacheT PhysicsObject::mMeshCache;

PhysicsObject::PhysicsObject(EventLoop* loop,
                             dWorldID worldId,
                             dSpaceID spaceId,
                             const Location& location) :
Emitter(loop), 
mRootModule(NULL_HANDLE),
mForce(v3::ZERO),
mTorque(v3::ZERO),
mBodyOffset(v3::ZERO)
{
	mOdeBody = dBodyCreate(worldId);
	mSpaceId = dHashSpaceCreate(spaceId);
	dMassSetZero(&mOriginalMass);

	setPosition(location.position);
	setOrientation(location.orientation);
}

PhysicsObject::~PhysicsObject()
{
	GeomMapT::iterator it = mGeometry.begin();
	for (; it != mGeometry.end(); ++it)
		dGeomDestroy(it->second.geomId);

	dBodyDestroy(mOdeBody);
	dSpaceDestroy(mSpaceId);
	
	unregisterEvents();
}

std::string odeVectorOut(const dReal* v)
{
	std::stringstream ss;
	ss << "{ ";
	ss << v[0] << ", ";
	ss << v[1] << ", ";
	ss << v[2] << ", ";
	ss << v[3] << " }";

	return ss.str();
}

std::string odeMatrixOut(const dReal* m)
{
	std::stringstream ss;
	ss << "[\n";
	ss << "\t" << odeVectorOut(m) << "\n";
	ss << "\t" << odeVectorOut(m + 4) << "\n";
	ss << "\t" << odeVectorOut(m + 8) << "\n";
	ss << " ]";
	return ss.str();
}

void PhysicsObject::addModule(GameHandle moduleHandle,
                              const std::string& meshName,
                              ID::CollisionMode collisionMode,
                              const v3& position,
                              const qv4& orientation,
                              f32 density)
{
	MeshCacheT::const_iterator it = mMeshCache.find(meshName);
	if (it == mMeshCache.end())
	{
		boost::shared_ptr<OdeTriMesh> ptr(new OdeTriMesh(View::loadMesh(meshName)));
		mMeshCache[meshName] = ptr;
	}

	dGeomID odeGeometry = dCreateTriMesh
	(
		mSpaceId,
		mMeshCache[meshName]->getTriMesh(),
		NULL, 
		NULL,
		NULL
	);

	if (!odeGeometry)
	{
		errlog << "Create ode geometry failed for " << moduleHandle << "!";
		throw std::logic_error("Unable to create ODE geom!");
	}
	
	GameHandle* ptr = new GameHandle(moduleHandle);
 	dGeomSetData(odeGeometry, reinterpret_cast<void*>(ptr));

	dMass geomMass; 
	dMassSetTrimesh(&geomMass, density, odeGeometry);

// 	dbglog << "GeomMass: " << geomMass.mass;
// 	dbglog << "GeomCenter: " << odeVectorOut(geomMass.c);
// 	dbglog << "GeomInertia: " << odeMatrixOut(geomMass.I);

	dBodyGetMass(mOdeBody, &mOriginalMass);

//  dbglog << "BodyMass: " << mOriginalMass.mass;
//  dbglog << "BodyCenter: " << odeVectorOut(mOriginalMass.c);
//  dbglog << "BodyInertia: " << odeMatrixOut(mOriginalMass.I);

	dMassAdd(&mOriginalMass, &geomMass);

	dMassTranslate(&mOriginalMass, -mOriginalMass.c[0], -mOriginalMass.c[1], -mOriginalMass.c[2]);
	dBodySetMass(mOdeBody, &mOriginalMass);

	dGeomSetBody(odeGeometry, mOdeBody);

	dBodyGetMass(mOdeBody, &mOriginalMass);

//  dbglog << "NewBodyMass: " << mOriginalMass.mass;
//  dbglog << "NewCenter: " << odeVectorOut(mOriginalMass.c);
//  dbglog << "NewInertia: " << odeMatrixOut(mOriginalMass.I);

	bool isRootModule = mGeometry.empty();
	
	assert(mGeometry.find(moduleHandle) == mGeometry.end() && 
		"Module Handle already in geometry map!");

	mGeometry[moduleHandle].geomId = odeGeometry;
	mGeometry[moduleHandle].density = density;
	setCollisionModeGeom(moduleHandle, collisionMode);

	if (isRootModule)
	{
		mRootModule = moduleHandle;
		
		// It isn't possible to register events (with a destination GameHandle)
		// before a root Module has been attached.
		registerEvents();
	}
	else
	{
// 		infolog << "SetOffsetPosition: " << position;
		setOffsetPosition(moduleHandle, position);
		setOffsetOrientation(moduleHandle, orientation);
	}
	
	notifyControlAboutChangeInMass();
}

void PhysicsObject::addModule(boost::shared_ptr<PhysicsObject> po,
                              const v3& position,
                              const qv4& orientation)
{
	po->mBodyOffset = position;

	GeomMapT::const_iterator it = po->mGeometry.begin();
	for (; it != po->mGeometry.end(); ++it)
	{
		// Calculate new relative position
		dGeomID geom = it->second.geomId;
		const dReal* offset = dGeomGetOffsetPosition(geom);
		
		v3 newPos(offset);
		newPos += position;

		// Attach
		dGeomSetBody(geom, mOdeBody);
		dGeomSetOffsetPosition(geom, newPos.x, newPos.y, newPos.z);
		dSpaceRemove(po->mSpaceId, geom);
		dSpaceAdd(mSpaceId, geom);
	}
	
	// add the two masses
	dMass otherMass; 
	dBodyGetMass(po->mOdeBody, &otherMass);

// 	dbglog << "OtherMass: " << otherMass.mass;
// 	dbglog << "OtherCenter: " << odeVectorOut(otherMass.c);
// 	dbglog << "OtherInertia: " << odeMatrixOut(otherMass.I);

	dBodyGetMass(mOdeBody, &mOriginalMass);

// 	dbglog << "OwnMass: " << mOriginalMass.mass;
// 	dbglog << "OwnCenter: " << odeVectorOut(mOriginalMass.c);
// 	dbglog << "OwnInertia: " << odeMatrixOut(mOriginalMass.I);

	dMassAdd(&mOriginalMass, &otherMass);

	dMassTranslate(&mOriginalMass, -mOriginalMass.c[0], -mOriginalMass.c[1], -mOriginalMass.c[2]);
	dBodySetMass(mOdeBody, &mOriginalMass);

// 	dbglog << "NewMass: " << mOriginalMass.mass;
// 	dbglog << "NewCenter: " << odeVectorOut(mOriginalMass.c);
// 	dbglog << "NewInertia: " << odeMatrixOut(mOriginalMass.I);

	// Disable old body
	dBodyDisable(po->mOdeBody);
	
	notifyControlAboutChangeInMass();
}

void PhysicsObject::removeModule(boost::shared_ptr<PhysicsObject> po,
                                 const v3& position,
                                 const qv4& orientation)
{
	// Restore geoms to body
	BOOST_FOREACH(GeomMapT::value_type& vt, po->mGeometry)
	{
		// Calculate new relative position
		dGeomID geom = vt.second.geomId;
		const dReal* offset = dGeomGetOffsetPosition(geom);

		v3 newPos(offset);
		newPos -= po->mBodyOffset;

		// Attach (and detach from this)
		dGeomSetBody(geom, po->mOdeBody);
		dGeomSetOffsetPosition(geom, newPos.x, newPos.y, newPos.z);
		dSpaceRemove(mSpaceId, geom);
		dSpaceAdd(po->mSpaceId, geom);
		
		// Restore individual CollisionModes
		po->setCollisionModeGeom(vt.first, vt.second.collisionMode);
	}
	
	po->mBodyOffset = v3::ZERO;
	po->recalculateMass();
	po->setPosition(position);
	po->setOrientation(orientation);
	
	// Apply current velocities to other object
	const dReal* linearVel = dBodyGetLinearVel(mOdeBody);
	dBodySetLinearVel(po->mOdeBody, linearVel[0], linearVel[1], linearVel[2]);
	
	dBodyEnable(po->mOdeBody);

	recalculateMass();
}

void PhysicsObject::sendFullSync() const
{
	m3x3 inertia;
	getTotalInertia(inertia);

	FullSyncData fsd
	(
		getPosition(),
		getOrientation(),
		getVelocity(),
		getVelocityRelative(),
		getRotationVelocity(),
		getRotationVelocityRelative(),
		getTotalWeight().value(),
		inertia
	);
	
	emit<Physics::Event>(ID::PE_FULL_SYNC, fsd, mRootModule);
}

void PhysicsObject::debugOutput(std::string& output) const
{
	std::stringstream ss;

	ss << "\n";
	ss << "PhysicsObject:\n";
	ss << "Root Module: " << mRootModule << "\n";
	ss << "Body Offset: " << mBodyOffset << "\n";

	// Print Body Data
	dMass mass;
	dBodyGetMass(mOdeBody, &mass);
	ss << "\n";
	ss << "Body:\n";
	ss << "ID: " << mOdeBody << "\n";
	ss << "Enabled: " << (dBodyIsEnabled(mOdeBody) ? "Yes" : "No") << "\n";
	ss << "Mass: " << mass.mass << "\n";
	ss << "Center: " << odeVectorOut(mass.c) << "\n";
	ss << "Inertia: " << odeMatrixOut(mass.I) << "\n";
	ss << "Position: " << odeVectorOut(dBodyGetPosition(mOdeBody)) << "\n";
	ss << "Orientation: " << odeVectorOut(dBodyGetQuaternion(mOdeBody)) << "\n";
	ss << "LinearVel: " << odeVectorOut(dBodyGetLinearVel(mOdeBody)) << "\n";
	ss << "AngularVel: " << odeVectorOut(dBodyGetAngularVel(mOdeBody)) << "\n";
	ss << "LinearDamp: " << dBodyGetLinearDamping(mOdeBody) << "\n";
	ss << "AngularDamp: " << dBodyGetAngularDamping(mOdeBody) << "\n";
	ss << "NumJoints: " << dBodyGetNumJoints(mOdeBody) << "\n";
	ss << "WorldID: " << dBodyGetWorld(mOdeBody) << "\n";
	ss << "Flags: " << mOdeBody->flags << "\n";

	ss << "\n";
	ss << "Geoms:\n";
	dGeomID gIt = dBodyGetFirstGeom(mOdeBody);
	
	// No Geoms at all?
	if (gIt == 0)
	{
		ss << "<NONE>\n";
	}

	// Print Geom Data
	int num = 0;
	while(gIt != 0 && ++num)
	{
		dReal aabb[6];

		dQuaternion quat;
		ss << "Number:" << num << "\n";
		ss << "ID: " << gIt << "\n";
		ss << "Enabled: " << (dGeomIsEnabled (gIt) ? "Yes" : "No") << "\n";
		ss << "IsSpace: " << (dGeomIsSpace (gIt) ? "Yes" : "No") << "\n";
		ss << "BodyID: " << dGeomGetBody (gIt) << "\n";
		ss << "Position: " << odeVectorOut(dGeomGetPosition (gIt)) << "\n";
		dGeomGetQuaternion (gIt, quat);
		ss << "Orientation: " << odeVectorOut(quat) << "\n";
		ss << "OffsetPos: " << odeVectorOut(dGeomGetOffsetPosition (gIt)) << "\n";
		dGeomGetOffsetQuaternion (gIt, quat);
		ss << "OffsetOri: " << odeVectorOut(quat) << "\n";
		ss << "Flags: " << gIt->gflags << "\n";
		dGeomGetAABB(gIt, aabb);
		ss << "AABB (minx, maxx, miny, maxy, minz, maxz):\n";
		ss << "(" 
		   << aabb[0] << ", "
		   << aabb[1] << ", "
		   << aabb[2] << ", "
		   << aabb[3] << ", "
		   << aabb[4] << ", "
		   << aabb[5] << ")\n";

		ss << "\n";

		gIt = dBodyGetNextGeom (gIt);
	}

	output = ss.str();
}

void PhysicsObject::setPosition(const v3& pos)
{
	dBodySetPosition(mOdeBody, pos.x, pos.y, pos.z);
	
	emit<Physics::Event>(ID::PE_POSITION, pos, mRootModule);
}

void PhysicsObject::setOffsetPosition(GameHandle moduleHandle, const v3& pos)
{
	dGeomSetOffsetPosition(mGeometry[moduleHandle].geomId, pos.x, pos.y, pos.z);
}

void PhysicsObject::setOrientation(const qv4& rot )
{
	if (equals(rot, qv4::ZERO))
	{
		assert(! "Quaternion must not be (0,0,0,0)!");
	}

	dBodySetQuaternion(mOdeBody, rot.ptr());

	emit<Physics::Event>(ID::PE_ORIENTATION, rot, mRootModule);
}

void PhysicsObject::setOffsetOrientation(GameHandle moduleHandle, const qv4& rot)
{
	if (equals(rot, qv4::ZERO))
	{
		assert(! "Quaternion must not be (0,0,0,0)!");
	}

	dGeomSetOffsetQuaternion(mGeometry[moduleHandle].geomId, rot.ptr());
}

v3 PhysicsObject::getPosition() const
{
	return v3(dBodyGetPosition(mOdeBody)); 
}

qv4 PhysicsObject::getOrientation() const
{
	qv4 tempQuat = qv4::IDENTITY;
	const dReal* buffer = dBodyGetQuaternion(mOdeBody);
	return qv4(buffer[0], buffer[1], buffer[2], buffer[3]);
}

v3 PhysicsObject::getVelocity() const
{
	return v3(dBodyGetLinearVel(mOdeBody));
}

v3 PhysicsObject::getVelocityRelative() const
{
	const v3 fVel = getVelocity();
	v3 vPos;

	dBodyVectorFromWorld(mOdeBody, fVel.x, fVel.y, fVel.z, vPos.ptr());

	return v3(vPos);	
}

v3 PhysicsObject::getRotationVelocity() const
{
	return v3(dBodyGetAngularVel(mOdeBody));
}

v3 PhysicsObject::getRotationVelocityRelative() const
{
	const v3 fVel = getRotationVelocity();
	v3 vPos;
	dBodyVectorFromWorld(mOdeBody, fVel.x, fVel.y, fVel.z, vPos.ptr());

	return v3(vPos);	
}

void PhysicsObject::setCollisionMode(ID::CollisionMode cm)
{
	BOOST_FOREACH(GeomMapT::value_type& vt, mGeometry)
	{
		setCollisionModeGeom(vt.first, cm);
	}
}

ID::CollisionMode PhysicsObject::getCollisionMode(GameHandle moduleHandle) const
{
	GeomMapT::const_iterator it = mGeometry.find(moduleHandle);
	if (it == mGeometry.end())
	{
		std::stringstream ss;
		ss << "Module/Geom "
		   << moduleHandle
		   << " not found within this PhysicsObject";
		throw std::logic_error(ss.str());
	}
	return it->second.collisionMode;
}

bool PhysicsObject::hasModule(GameHandle moduleHandle) const
{
	return mGeometry.find(moduleHandle) != mGeometry.end();
}

void PhysicsObject::enableSimulation(bool enable)
{
	if (enable)
		dBodyEnable(mOdeBody);
	else
		dBodyDisable(mOdeBody);
}

// \bug new mass differs from the old to some degree.
void PhysicsObject::recalculateMass()
{
	dMassSetZero(&mOriginalMass);

	GeomMapT::iterator it = mGeometry.begin();
	for (; it != mGeometry.end(); ++it)
	{
		//! \bug 
		//! The real inertia-matrix is asymetrical and makes it difficult
		//!	to find the right working point for the rotational forces.
		//!	So we use a symetrical matrix by removing the offset and adding it
		//! again after the mass calculation. If you ever fix this bug don't 
		//! forget to fix attachModule which calculates a wrong symetrical 
		//! matrix too.

		dVector3 offsetPos;
		dQuaternion offsetOri;

		dGeomCopyOffsetPosition(it->second.geomId, offsetPos);
		dGeomGetOffsetQuaternion(it->second.geomId, offsetOri);

		dGeomClearOffset(it->second.geomId);

		dMass geomMass; 
		dMassSetTrimesh(&geomMass, it->second.density, it->second.geomId);

		dbglog << "ID: " << it->second.geomId << " Mass: " << geomMass.mass;
		dMassAdd(&mOriginalMass, &geomMass);

		dGeomSetOffsetPosition(it->second.geomId, offsetPos[0], offsetPos[1], offsetPos[2]);
		dGeomSetOffsetQuaternion(it->second.geomId, offsetOri);
	}

	dMassTranslate(&mOriginalMass, -mOriginalMass.c[0], -mOriginalMass.c[1], -mOriginalMass.c[2]);

	dBodySetMass(mOdeBody, &mOriginalMass);
	notifyControlAboutChangeInMass();
}

void PhysicsObject::prepareOdeStep(quantity<si::time, f32> stepsize)
{
	dBodyAddRelForce(mOdeBody, mForce.x, mForce.y, mForce.z);
	dBodyAddRelTorque(mOdeBody, mTorque.x, mTorque.y, mTorque.z);
}

void PhysicsObject::clearForces()
{
	mForce = mTorque = v3::ZERO;
}

void PhysicsObject::magicStop()
{
	dBodySetLinearVel(mOdeBody, 0, 0, 0);
	dBodySetAngularVel(mOdeBody, 0, 0, 0);
	dBodySetForce(mOdeBody, 0, 0, 0);
	dBodySetTorque(mOdeBody, 0, 0, 0);
}

void PhysicsObject::modulateMass(f32 factor) const
{
	//! \note This could be optimized, since Ode internally recalculates our
	//!       factor (in dMassAdjust).
	dMass newMass = mOriginalMass;        // don't modify the original value
	f32 weight = newMass.mass * factor;   // calculate new weight for adjust()
	newMass.adjust(weight);
	dBodySetMass(mOdeBody, &newMass);
	
	notifyControlAboutChangeInMass();
}

quantity<si::mass, f32> PhysicsObject::getTotalWeight() const
{
	dMass mass;
	dBodyGetMass(mOdeBody, &mass);
	return mass.mass * si::kilogram;
}

void PhysicsObject::notifyControlAboutChangeInMass() const
{
	emit<Event>
	(
		ID::PE_TOTAL_MASS,
		getTotalWeight().value(),
		mRootModule
	);
}

void PhysicsObject::getTotalInertia(m3x3& inertia) const
{
	dMass mass;
	dBodyGetMass(mOdeBody, &mass);

	inertia = m3x3
	(
		mass.I[0],
		mass.I[1],
		mass.I[2],
		mass.I[4],
		mass.I[5],
		mass.I[6],
		mass.I[8],
		mass.I[9],
		mass.I[10]
	);
}

void PhysicsObject::registerEvents()
{
	mPhysicsEvents.push_back(ID::PE_UPDATE_POSITION);
	mPhysicsEvents.push_back(ID::PE_UPDATE_ORIENTATION);

	mPhysicsEvents.push_back(ID::PE_UPDATE_VELOCITY);
	mPhysicsEvents.push_back(ID::PE_UPDATE_ROTATION_VELOCITY);
	
	mPhysicsEvents.push_back(ID::PE_DEBUG);
		
	mPhysicsEvents.push_back(ID::PE_APPLY_FORCE);
	mPhysicsEvents.push_back(ID::PE_APPLY_TORQUE);
	mPhysicsEvents.push_back(ID::PE_MODULATE_MASS);
	
	BOOST_FOREACH(BFG::ID::PhysicsAction action, mPhysicsEvents)
	{
		loop()->connect(action, this, &PhysicsObject::eventHandler, mRootModule);
	}
}

void PhysicsObject::unregisterEvents()
{
	BOOST_FOREACH(BFG::ID::PhysicsAction action, mPhysicsEvents)
	{
		loop()->disconnect(action, this);
	}
	
	mPhysicsEvents.clear();
}

void PhysicsObject::eventHandler(Physics::Event* event)
{
	switch (event->getId())
	{
	case ID::PE_UPDATE_POSITION:
		setPosition(boost::get<v3>(event->getData()));
		break;

	case ID::PE_UPDATE_ORIENTATION:
		setOrientation(boost::get<qv4>(event->getData()));
		break;
		
	case ID::PE_UPDATE_VELOCITY:
		setVelocity(boost::get<v3>(event->getData()));
		break;

	case ID::PE_UPDATE_ROTATION_VELOCITY:
		setRotationVelocity(boost::get<v3>(event->getData()));
		break;

	case ID::PE_DEBUG:
	{
		std::string s;
		debugOutput(s);
		dbglog << s;
		break;
	}

	case ID::PE_APPLY_FORCE:
		onForce(boost::get<v3>(event->getData()));
		break;

	case ID::PE_APPLY_TORQUE:
		onTorque(boost::get<v3>(event->getData()));
		break;
		
	case ID::PE_MODULATE_MASS:
		modulateMass(boost::get<f32>(event->getData()));
		break;

	default:
		throw std::logic_error
			("PhysicsObject::eventHandler: Received unknown event!");
	}
}

void PhysicsObject::setVelocity(const v3& velocity)
{
	dBodySetLinearVel(mOdeBody, velocity.x, velocity.y, velocity.z);
}

void PhysicsObject::setRotationVelocity(const v3& rotVelocity)
{
	dBodySetAngularVel(mOdeBody, rotVelocity.x, rotVelocity.y, rotVelocity.z);
}

void PhysicsObject::setCollisionModeGeom(GameHandle moduleHandle,
                                         ID::CollisionMode cm)
{
	GeomMapT::iterator it = mGeometry.find(moduleHandle);
	assert(it != mGeometry.end() && "I hope you'll never read this.");
	dGeomID geom = it->second.geomId;

	switch (cm)
	{
		case ID::CM_Disabled:
			dGeomDisable(geom);
			it->second.collisionMode = ID::CM_Disabled;
			break;
		
		case ID::CM_Standard:
			dGeomEnable(geom);
			it->second.collisionMode = ID::CM_Standard;
			break;
			
		case ID::CM_Ghost:
			dGeomEnable(geom);
			it->second.collisionMode = ID::CM_Ghost;
			break;
			
		default:
			throw std::logic_error
				("PhysicsObject::setCollisionModeGeom: Unknown CollisionMode"); 
			break;
	}
}

void PhysicsObject::onForce(const v3& force)
{
	mForce = force;
}

void PhysicsObject::onTorque(const v3& torque)
{
	mTorque = torque;
}

std::ostream& operator << (std::ostream& lhs, const PhysicsObject& rhs)
{
	std::string s;
	rhs.debugOutput(s);
	lhs << s;
	return lhs;
}

} // namespace Physics
} // namespace BFG
