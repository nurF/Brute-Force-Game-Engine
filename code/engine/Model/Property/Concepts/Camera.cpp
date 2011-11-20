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

#include <Model/Property/Concepts/Camera.h>

#include <stdexcept>

#include <Model/Environment.h>
#include <Model/Module.h>
#include <Model/GameObject.h>

#include <Physics/Event.h>

namespace BFG {

Camera::Camera(GameObject& owner, PluginId pid) :
Property::Concept(owner, "Camera", pid),
mTarget(NULL_HANDLE)
#if 0
mMode(parameter.mMode),
mMaxRotationVelocity(parameter.mMaxRotationVelocity),
mMaxSpeed(parameter.mMaxSpeed),
mWaypoint(parameter.mWPLocation),
mOffset(parameter.mOffset),
mReactionTime(parameter.mReactionTime),
mMaxDistance(parameter.mMaxDistance),
mTargetOffset(parameter.mTargetOffset),
mStiffness(parameter.mStiffness),
mOwnLocation(),
mNeedsUpdate(false),
mRotationInput(v3::ZERO),
mSpeedInput(0.0f * si::meters_per_second),
mOffsetFactor(1.0f),
mOffsetChange(0),
mInterpol(0.0f, parameter.mMaxDistance.value())
#endif
{
#if 0
	requestEvent(ID::GOE_CONTROL_PITCH);
	requestEvent(ID::GOE_CONTROL_YAW);
	requestEvent(ID::GOE_CONTROL_ROLL);
	requestEvent(ID::GOE_CONTROL_THRUST);
	requestEvent(ID::GOE_CONTROL_MAGIC_STOP);
	requestEvent(ID::GOE_TOGGLE_WIREFRAME);
#endif
	requestEvent(ID::GOE_SET_CAMERA_TARGET);
	initvar(ID::PV_CameraMode);
	require("Physical");

	assert(ownerHandle() == rootModule() &&
		"Camera: This code may contain some out-dated assumptions.");
}

Camera::~Camera()
{
	//! \todo This needs to be done when this module is actually "switched off".
#if 0
	stopDelivery(ID::GOE_CONTROL_PITCH);
	stopDelivery(ID::GOE_CONTROL_YAW);
	stopDelivery(ID::GOE_CONTROL_ROLL);
	stopDelivery(ID::GOE_CONTROL_THRUST);
	stopDelivery(ID::GOE_CONTROL_MAGIC_STOP);
	stopDelivery(ID::GOE_TOGGLE_WIREFRAME);
#endif
	stopDelivery(ID::GOE_SET_CAMERA_TARGET);
}

void Camera::internalOnEvent(EventIdT action,
                             Property::Value payload,
                             GameHandle module,
                             GameHandle sender)
{
	switch(action)
	{
	case ID::GOE_SET_CAMERA_TARGET:
	{
		mTarget = payload;
		break;
	}

#if 0
	case ID::GOE_CONTROL_PITCH:
	{
		// rotation around x-axis
		mRotationInput.x += payload * mMaxRotationVelocity.x;
		mRotationInput = clamp(mRotationInput, -mMaxRotationVelocity, mMaxRotationVelocity);
		mNeedsUpdate = true;
		break;
	}

	case ID::GOE_CONTROL_YAW:
	{
		// rotation around y-axis
		mRotationInput.y += payload * mMaxRotationVelocity.y;
		mRotationInput = clamp(mRotationInput, -mMaxRotationVelocity, mMaxRotationVelocity);
		mNeedsUpdate = true;
		break;
	}

	case ID::GOE_CONTROL_ROLL:
	{
		mRotationInput.z += payload * mMaxRotationVelocity.z;
		mRotationInput = clamp(mRotationInput, -mMaxRotationVelocity, mMaxRotationVelocity);
		mNeedsUpdate = true;
		break;
	}

	case ID::GOE_CONTROL_THRUST:
	{
		switch(mMode)
		{
		case ID::CM_Free:
			mSpeedInput += payload * mMaxSpeed;
			mSpeedInput = clamp(mSpeedInput, -mMaxSpeed, mMaxSpeed);
			mNeedsUpdate = true;
			break;
		case ID::CM_Fixed:
		case ID::CM_Chase:
			mOffsetChange = static_cast<s32>(static_cast<f32>(payload));
			break;
		}
		break;
	}

	case ID::GOE_CONTROL_MAGIC_STOP:
		mOwnLocation.orientation = qv4::IDENTITY;
		mOffsetFactor = 1.0f;
		break;

	case ID::GOE_TOGGLE_WIREFRAME:
		View::RenderInterface::Instance()->toggleWireframe();
		break;
#endif

	default:
		throw std::logic_error("Model::Camera::internalOnEvent: Unknown Action.");
	}
}

void Camera::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	assert(mModules.begin()->first == rootModule() &&
		"Camera: Must be in charge of the root module!");

	(this->*specificUpdate)(timeSinceLastFrame);
}

void Camera::internalOnModuleAttached(GameHandle module)
{
	assert(mModules.size() == 1 && 
		"Camera: only one Camera allowed at the moment!");

	mMode = static_cast<ID::CameraMode>(value<s32>(ID::PV_CameraMode, module));

	setMode(mMode);
	
	mOffset = value<v3>(ID::PV_CameraOffset, module);
}

void Camera::updateOwnLocation(quantity<si::time, f32> timeSinceLastFrame)
{
#if 0
	mRotationInput *= timeSinceLastFrame.value();

	qv4 oriP(Ogre::Radian(mRotationInput.x), v3::UNIT_X);
	qv4 oriY(Ogre::Radian(mRotationInput.y), v3::UNIT_Y);
	qv4 oriR(Ogre::Radian(mRotationInput.z), v3::UNIT_Z);
	
	mOwnLocation.orientation = mOwnLocation.orientation * oriP * oriY * oriR;

	if(mOffsetChange > 0)
	{
		mOffsetFactor += 0.2f * timeSinceLastFrame.value();
		mOffsetChange = 0;
	}
	else if(mOffsetChange < 0)
	{
		mOffsetFactor -= 0.2f * timeSinceLastFrame.value();
		mOffsetChange = 0;
	}
#endif
}


void Camera::reset()
{
	mNeedsUpdate = false;
#if 0
	mRotationInput = v3::ZERO;
	mSpeedInput = 0.0f * si::meters_per_second;
#endif
}

void Camera::setMode(ID::CameraMode mode)
{
	switch(mode)
	{
#if 0
	case ID::CM_Free:
		specificUpdate = &Camera::updateFree;
		break;
#endif
	case ID::CM_Fixed:
		specificUpdate = &Camera::updateFixed;
		break;
#if 0
	case ID::CM_Chase:
		specificUpdate = &Camera::updateChase;
		break;
	case ID::CM_ChaseTarget:
		specificUpdate = &Camera::updateChaseTarget;
		break;
#endif
	default:
		throw std::logic_error("CameraMode ID not recognized!");
	}

	mMode = mode;
}

#if 0

void Camera::updateFree(quantity<si::time, f32> timeSinceLastFrame)
{
	if (mNeedsUpdate)
	{
		updateOwnLocation(timeSinceLastFrame);

		v3 pos = mOwnLocation.orientation.zAxis();
		pos.normalise();

		mOwnLocation.position += pos * (mSpeedInput * timeSinceLastFrame).value();

		Physics::PhysicalObjInterface::Instance()->setOrientation(ownerHandle(), mOwnLocation.orientation);
		Physics::PhysicalObjInterface::Instance()->setPosition(ownerHandle(), mOwnLocation.position);
		reset();
	}
}

#endif

void Camera::updateFixed(quantity<si::time, f32> timeSinceLastFrame)
{
	Location target;

	if (! environment()->exists(mTarget))
		return;

	if (mTarget != NULL_HANDLE)
		target = environment()->getGoValue<Location>(mTarget, ID::PV_Location, pluginId());

	updateOwnLocation(timeSinceLastFrame);

	qv4 newOri = target.orientation * mOwnLocation.orientation;

	v3 newPos = target.position + (newOri * mOffset);
#if 0
	v3 newPos = target.position + (newOri * (mOffset * mOffsetFactor));
#endif

	emit<Physics::Event>(ID::PE_UPDATE_ORIENTATION, newOri, ownerHandle());
	emit<Physics::Event>(ID::PE_UPDATE_POSITION, newPos, ownerHandle());
}

#if 0

void Camera::updateChase(quantity<si::time, f32> timeSinceLastFrame)
{
	updateOwnLocation(timeSinceLastFrame);

	qv4 ori = mWaypoint->mLocation.orientation * mOwnLocation.orientation;
 	ori = ori.nlerp(mStiffness * timeSinceLastFrame.value(), mLastObjLocation.orientation, ori, true);
	mLastObjLocation.orientation = ori;

	v3 objVel = (mWaypoint->mLocation.position - mLastObjLocation.position) / timeSinceLastFrame.value();
	v3 objAcc = (objVel - mLastObjVelocity) / timeSinceLastFrame.value();

	
	// just get the acceleration of the current z-axis
	f32 acc = (ori.UnitInverse() * objAcc).z;
	
	if (acc > 0.1f)
	{
		// if there is acceleration mFactor will be increase so that it reaches 1.0f in mReactionTime seconds
		mFactor += timeSinceLastFrame / mReactionTime; 
	}
	else 
	{
		// return to 0 if there is no acceleration
		mFactor -= timeSinceLastFrame / mReactionTime;
	}

	mFactor = clamp(mFactor.value(), 0.0f, 1.0f);

	// ease in ease out interpolation till the maxDistance is reached.
	f32 distance = mInterpol.Interpolate(mFactor);
	
	mLastObjVelocity = objVel;
	mLastObjLocation.position = mWaypoint->mLocation.position;

	Physics::PhysicalObjInterface::Instance()->setOrientation(ownerHandle(), mLastObjLocation.orientation);
	Physics::PhysicalObjInterface::Instance()->setPosition(
	    ownerHandle(), 
	    mLastObjLocation.position - ori.zAxis() * distance + (ori * (mOffset * mOffsetFactor)));
}

void Camera::updateChaseTarget( quantity<si::time, f32> timeSinceLastFrame )
{
	qv4 ori = mWaypoint->mLocation.orientation;
	ori = ori.nlerp(mStiffness * timeSinceLastFrame.value(), mLastObjLocation.orientation, ori, true);
	mLastObjLocation.orientation = ori;

	v3 transformedOffset(ori * mOffset);
	mOwnLocation.position = mWaypoint->mLocation.position + transformedOffset ;

	// Note: there probably is a problem when multiplying with a unified vector.
	//       I rotated a unit_Z vector by exactly 90° (via quaternion) and it returned
	//       (1, 0, 5.472e-008). With a non-unified vector there was no noticable
	//       inprecision. 
	//       Old code was like this:
	//       v3 targetVector(mWaypoint->mLocation.orientation * v3::UNIT_Z);
	//       targetVector *= mTargetOffset.value();
	v3 targetVector(mWaypoint->mLocation.orientation * v3(0, 0, mTargetOffset.value()));

  	v3 vectorToTarget(targetVector - transformedOffset);

	qv4 oriV2T = vectorToTarget.getRotationTo(mWaypoint->mLocation.orientation.zAxis(), mWaypoint->mLocation.orientation.xAxis());

	mOwnLocation.orientation = mWaypoint->mLocation.orientation * oriV2T;
	Physics::PhysicalObjInterface::Instance()->setOrientation(ownerHandle(), mOwnLocation.orientation);
	Physics::PhysicalObjInterface::Instance()->setPosition(ownerHandle(), mOwnLocation.position);

}
#endif

} // namespace BFG
