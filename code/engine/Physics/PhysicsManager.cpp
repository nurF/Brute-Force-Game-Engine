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

#include <Physics/PhysicsManager.h>

#include <sstream>

#include <boost/array.hpp>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <Base/CLogger.h>
#include <Base/Cpp.h>

#include <Model/Events/GameObjectEvent.h>

#include <Physics/OdeTriMesh.h>
#include <Physics/PhysicsObject.h>

namespace BFG {
namespace Physics {

const u32 g_AbsoluteMaximumContactsPerCollision = 512;

void globalOdeNearCollisionCallback(void* additionalData,
                                    dGeomID geo1,
                                    dGeomID geo2)
{
	assert(additionalData);
	PhysicsManager* This = static_cast<PhysicsManager*>(additionalData);
	This->onNearCollision(geo1, geo2);
}

static void odeErrorHandling(int errnum, const char* msg, va_list ap)
{
	boost::array<char, 512> formatted_msg;

	vsnprintf(formatted_msg.data(), formatted_msg.size(), msg, ap);

	errlog << "ODE: " << formatted_msg.data();
}

static void odeMessageHandling(int errnum, const char* msg, va_list ap)
{
	boost::array<char, 512> formatted_msg;

	vsnprintf(formatted_msg.data(), formatted_msg.size(), msg, ap);

	infolog << "ODE: " << formatted_msg.data();
}

static void odeDebugHandling(int errnum, const char* msg, va_list ap)
{
	boost::array<char, 512> formatted_msg;

	vsnprintf(formatted_msg.data(), formatted_msg.size(), msg, ap);

	dbglog << "ODE: " << formatted_msg.data();
}


PhysicsManager::PhysicsManager(EventLoop* loop, u32 maxContactsPerCollision) :
Emitter(loop),
mMaxContactsPerCollision(maxContactsPerCollision),
mSimulationStepSize(0.001f * si::seconds),
mWorldID(0),
mHashSpaceID(0),
mCollisionJointsID(0)
{
	assert(maxContactsPerCollision <= g_AbsoluteMaximumContactsPerCollision);

	dInitODE();

	dSetMessageHandler(odeMessageHandling);
	dSetErrorHandler(odeErrorHandling);
	dSetDebugHandler(odeDebugHandling);

	mWorldID = dWorldCreate();
	
	mHashSpaceID = dHashSpaceCreate(0);
	
	mCollisionJointsID = dJointGroupCreate(0);

	// set ODE Gravity to zero
	dWorldSetGravity(mWorldID, 0.0, 0.0, 0.0f);
	dWorldSetCFM(mWorldID, 1e-5f);
	dWorldSetAutoDisableFlag(mWorldID, true);

	// set error reduction parameter 
	dWorldSetERP(mWorldID, 0.1f);

	registerEvents();

	infolog << "PhysicsManager initialized";
}

PhysicsManager::~PhysicsManager()
{
	unregisterEvents();

	// Ensure that this gets done before ODE is closed
	mPhysicsObjects.clear();

	dSpaceDestroy(mHashSpaceID);
	dJointGroupDestroy(mCollisionJointsID);
	dWorldDestroy(mWorldID);
	dCloseODE();

	infolog << "PhysicsManager destroyed";
}

dSpaceID PhysicsManager::getSpaceID() const
{
	return mHashSpaceID;
}

dWorldID PhysicsManager::getWorldID() const
{
	return mWorldID;
}

void PhysicsManager::clear()
{
	mPhysicsObjects.clear();
}

void PhysicsManager::move(quantity<si::time, f32> timeSinceLastFrame)
{
	// Time left from the previous frame (always less than mSimulationStepSize)
	static quantity<si::time, f32> timeLeft = timeSinceLastFrame;

	int nrofsteps = int(ceil(timeSinceLastFrame / mSimulationStepSize));

	/** \note
		On low FPS, time may accumulate which leads to serious timing problems.

		Example:
		- ODE may take a long time, then the next timeSinceLastFrame is higher
		- Due to the higher timeSinceLastFrame, Ode takes even longer, etc...
		
		Solutions:
		- At the moment, we're simply limiting the number of ode steps
		- Another solution would be to subtract the time ODE took, from
		  timeSinceLastFrame.
		- Additionally we might increase the mSimulationStepSize on low FPS.
	*/

	// Equal to 5 FPS
	const int maxSteps = 200;
	
	if (nrofsteps > maxSteps)
	{
		nrofsteps = maxSteps;
		timeLeft = 0;
	}

	for (int i=0; i<nrofsteps; i++)
	{
		PhysicsObjectMap::iterator it = mPhysicsObjects.begin();
		for (; it != mPhysicsObjects.end(); ++it)
		{
			it->second->prepareOdeStep(mSimulationStepSize);
		}

		/** \note
			The dSpaceCollide call, which performs collision detection, was
			previously done outside of this `for' loop, which caused collision
			detection errors the lower the framerate was.
			
			Of course, dSpaceCollide should be called after each world step.
			But this is very slow (at least on my machine :-P) so I call
			it every third step for now. For example, this is still better than
			calling it once for 10ms at 100FPS.
			
			Probably, we can optimize a lot here.
		*/
		if (i % 3 == 0)
			dSpaceCollide (mHashSpaceID, this, &globalOdeNearCollisionCallback);

		dWorldQuickStep (mWorldID, mSimulationStepSize.value());

		if (i % 3 == 0)
			dJointGroupEmpty (mCollisionJointsID);
		
		timeLeft -= mSimulationStepSize;
	}

	PhysicsObjectMap::iterator it = mPhysicsObjects.begin();
	for (; it != mPhysicsObjects.end(); ++it)
	{
		it->second->clearForces();
		it->second->sendFullSync();
	}
	
	
}

void PhysicsManager::addObject(GameHandle handle,
                               boost::shared_ptr<PhysicsObject> object)
{
	mPhysicsObjects[handle] = object;
}


#if 0
void PhysicsManager::createPhysicalObject(GameHandle handle,
										  ID::CollisionGeometry geomType, 
									      CollisionTypeData dat, 
									      ID::CollisionType colType,
									      bool collidable,
									      bool simulated)
{
	// geometry will be created depending on geomType
	dGeomID odeGeometry;

	switch (geomType)
	{
	case ID::CG_Sphere:
		{
			float radius = boost::get<float>(dat);
			odeGeometry = dCreateSphere(mHashSpaceID, radius);
			break;
		}
	case ID::CG_Box:
		{
			v3 BoxSideLength = boost::get<v3>(dat);
			odeGeometry = dCreateBox(mHashSpaceID, BoxSideLength.x, BoxSideLength.y, BoxSideLength.z);
			break;
		}
	case ID::CG_CappedCylinder:
		{
			//radius and length of cylinder
			std::pair<float, float> CylinderData = boost::get<std::pair<float, float> >(dat);
			odeGeometry = dCreateCCylinder(mHashSpaceID, CylinderData.first, CylinderData.second);
			break;
		}
	case ID::CG_Plane:
		{
			assert(! "createPhysicalObject(): Implement case for ID::CG_Plane");
			break;
		}
	case ID::CG_TriMesh:
		{
			std::string meshName = boost::get<std::string>(dat);
			boost::shared_ptr<OdeTriMesh> odeTriMesh = View::RenderInterface::Instance()->createOdeTriMesh(meshName);

			if (!odeTriMesh)
			{
				std::stringstream stringstr;
				stringstr << "Create odeTriMesh failed for " << meshName << "!";
				throw std::logic_error(stringstr.str());
			}

			odeGeometry = dCreateTriMesh(
				mHashSpaceID,
				odeTriMesh->getTriMesh(),
				NULL, 
				NULL,
				NULL
			);
			break;
		}
	}

	if (!odeGeometry)
	{
		std::stringstream stringstr;
		stringstr << "Create ode geometry failed for " << handle << "!";
		throw std::logic_error(stringstr.str());
	}

	dBodyID odeBody = dBodyCreate(mWorldID);

	if (!odeBody)
	{
		std::stringstream stringstr;
		stringstr << "Create ode body failed for " << handle << "!";
		throw std::logic_error(stringstr.str());
	}

	PhysicsObject* object = new PhysicsObject(handle, odeGeometry, odeBody, colType);
	mPhysicsObjects.insert(handle, object);
	
	if (collidable) object->enableCollision();
	else            object->disableCollision();
		
	if (simulated)  object->enableSimulation();
	else            object->disableSimulation();
}
#endif

void PhysicsManager::onNearCollision(dGeomID geo1, dGeomID geo2)
{
	if (dGeomIsSpace (geo1) || dGeomIsSpace (geo2))
	{
		dSpaceCollide2
		(
			geo1,
			geo2,
			this,
			&globalOdeNearCollisionCallback
		);
		return;
	}

	dBodyID b1 = dGeomGetBody(geo1);
	dBodyID b2 = dGeomGetBody(geo2);

	int ConnectedByJoint = dAreConnectedExcluding(b1, b2, dJointTypeContact);

	if (b1 && b2 && ConnectedByJoint == 1)
		return;

	// dCollide does even collide disabled geoms (unlike dSpaceCollide)
	if (!dGeomIsEnabled(geo1) || !dGeomIsEnabled(geo2))
		return;

	collideGeoms(geo1, geo2);
}

void PhysicsManager::collideGeoms(dGeomID geo1, dGeomID geo2) const
{
	dContact contact[g_AbsoluteMaximumContactsPerCollision];
	
	for (u32 i=0; i<mMaxContactsPerCollision; ++i) 
	{
		contact[i].surface.mode = dContactApprox1;

		// 0.1 (metal  on ice) to 0.9 (rubber on pavement), 
		contact[i].surface.mu = 0.25f;
		contact[i].surface.mu2 = 0;
	}

	int collisions = dCollide
	(
		geo1,
		geo2,
		mMaxContactsPerCollision,
		&contact[0].geom,
		sizeof(dContact)
	);
	
	if (collisions == 0)
		return;

	dBodyID b1 = dGeomGetBody(geo1);
	dBodyID b2 = dGeomGetBody(geo2);

	GameHandle moduleHandle1 = *reinterpret_cast<GameHandle*>(dGeomGetData(geo1));
	GameHandle moduleHandle2 = *reinterpret_cast<GameHandle*>(dGeomGetData(geo2));

	boost::shared_ptr<PhysicsObject> po1 = searchMovObj(moduleHandle1);
	boost::shared_ptr<PhysicsObject> po2 = searchMovObj(moduleHandle2);
	
	if (po1->getCollisionMode(moduleHandle1) == ID::CM_Ghost &&
	    po2->getCollisionMode(moduleHandle2) == ID::CM_Standard)
	{
		// Causes contact joints to be ignored. No effect for body TWO.
		b2 = NULL;
	}
	else if (po2->getCollisionMode(moduleHandle2) == ID::CM_Ghost &&
	         po1->getCollisionMode(moduleHandle1) == ID::CM_Standard)
	{
		// Causes contact joints to be ignored. No effect for body ONE.
		b1 = NULL;
	}

	for (int i=0; i<collisions; ++i) 
	{
		dJointID c = dJointCreateContact
		(
			mWorldID,
			mCollisionJointsID,
			contact+i
		);

		dJointAttach (c,b1,b2);
	}

	float totalPenetrationDepth = 0;
	
	for (int i=0; i<collisions; ++i)
		totalPenetrationDepth += contact[i].geom.depth;

	assert(moduleHandle1 && moduleHandle2);

	// If these pointers are valid, then a contact joint was attached and
	// an effect will be issued, therefore we can safely send a PE_CONTACT.
	if (b1)
	{
		//! Hack: Abusing Sender-ID as additional payload!
		emit<GameObjectEvent>
		(
			ID::PE_CONTACT,
			totalPenetrationDepth,
			moduleHandle1,
			moduleHandle2
		);
	}
	if (b2)
	{
		//! Hack: Abusing Sender-ID as additional payload!
		emit<GameObjectEvent>
		(
			ID::PE_CONTACT,
			totalPenetrationDepth,
			moduleHandle2,
			moduleHandle1
		);
	}
}

void PhysicsManager::registerEvents()
{
	mPhysicsEvents.push_back(ID::PE_STEP);
	mPhysicsEvents.push_back(ID::PE_CLEAR);
	mPhysicsEvents.push_back(ID::PE_CREATE_OBJECT);
	mPhysicsEvents.push_back(ID::PE_DELETE_OBJECT);
	mPhysicsEvents.push_back(ID::PE_ATTACH_MODULE);
	mPhysicsEvents.push_back(ID::PE_REMOVE_MODULE);

	BOOST_FOREACH(ID::PhysicsAction action, mPhysicsEvents)
	{
		loop()->connect(action, this, &PhysicsManager::eventHandler);
	}
}

void PhysicsManager::unregisterEvents()
{
	BOOST_FOREACH(ID::PhysicsAction action, mPhysicsEvents)
	{
		loop()->disconnect(action, this);
	}

	mPhysicsEvents.clear();
}

void PhysicsManager::eventHandler(Physics::Event* event)
{
	switch (event->getId())
	{
	case ID::PE_STEP:
		move(boost::get<f32>(event->getData()) * si::seconds);
		break;

	case ID::PE_CLEAR:
		clear();
		break;
		
	case ID::PE_CREATE_OBJECT:
		onCreateObject(boost::get<ObjectCreationParams&>(event->getData()));
		break;

	case ID::PE_DELETE_OBJECT:
		onDeleteObject(boost::get<GameHandle>(event->getData()));
		break;

	case ID::PE_ATTACH_MODULE:
		onAttachModule(boost::get<ModuleCreationParams&>(event->getData()));
		break;

	case ID::PE_REMOVE_MODULE:
		onRemoveModule(boost::get<ModuleRemovalParams&>(event->getData()));
		break;

	default:
		throw std::logic_error
			("PhysicsManager::eventHandler: Received unknown event!");
	}
}

void PhysicsManager::onCreateObject(const ObjectCreationParams& ocp)
{
	GameHandle handle = ocp.get<0>();
	const Location& location = ocp.get<1>();

	boost::shared_ptr<PhysicsObject> po
	(
		new PhysicsObject
		(
			loop(),
			getWorldID(),
			getSpaceID(),
			location
		)
	);
	addObject(handle, po);
	po->sendFullSync();
}

void PhysicsManager::onDeleteObject(GameHandle handle)
{
	PhysicsObjectMap::iterator it = mPhysicsObjects.find(handle);
	assert(it != mPhysicsObjects.end());
	mPhysicsObjects.erase(it);
}

void PhysicsManager::onAttachModule(const ModuleCreationParams& mcp)
{
	GameHandle poHandle = mcp.get<0>();

	PhysicsObjectMap::const_iterator it = mPhysicsObjects.find(poHandle);
	if (it == mPhysicsObjects.end())
	{
		std::stringstream ss;
		ss << "PhysicsObject " << poHandle << " not registered in Manager!";
		throw std::logic_error(ss.str());
	}
	
	boost::shared_ptr<PhysicsObject> po = it->second;

	po->addModule
	(
		mcp.get<1>(),
		mcp.get<2>(),
		mcp.get<3>(),
		mcp.get<4>(),
		mcp.get<5>(),
		mcp.get<6>()
	);
	po->sendFullSync();
}

void PhysicsManager::onRemoveModule(const ModuleRemovalParams& mcp)
{
	warnlog << "STUB: PhysicsManager::onRemoveModule(const ModuleRemovalParams& mcp)";
}

boost::shared_ptr<PhysicsObject>
PhysicsManager::searchMovObj(GameHandle handle) const
{
	boost::shared_ptr<PhysicsObject> result;
	BOOST_FOREACH(const PhysicsObjectMap::value_type& vt, mPhysicsObjects)
	{
		if (vt.second->hasModule(handle))
		{
			result = vt.second;
			break;
		}
	}

	if (! result)
	{
		std::stringstream ss;
		ss << "PhysicsObject " << handle << " not registered in Manager!";
		throw std::logic_error(ss.str());
	}

	return result;
}

} // namespace Physics
} // namespace BFG
