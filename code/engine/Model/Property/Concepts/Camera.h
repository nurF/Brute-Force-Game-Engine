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

#ifndef CAMERA_H_
#define CAMERA_H_


#include <boost/units/systems/si/velocity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/time.hpp>
#include <boost/units/systems/si/dimensionless.hpp>

#include <Core/Types.h>
#include <Base/CInterpolate.h>
#include <Model/Waypoint.h>
#include <Model/Property/Concept.h>

namespace BFG {

struct CameraParameter
{
	
	// standard constructor
	CameraParameter(ID::CameraMode mode = ID::CM_Fixed,
	                const v3& maxRot = v3(1.5708f),
	                f32 maxSpeed = 20.0f) :
	mMode(mode),
	mMaxRotationVelocity(maxRot),
	mMaxSpeed(maxSpeed * si::meters_per_second),
	mOffset(v3::ZERO),
	mReactionTime(0.0f * si::second),
	mMaxDistance(0.0f * si::meter),
	mTargetOffset(800.0f * si::meter),
	mFullscreen(true),
	mStiffness(2.8f) {}

	CameraParameter(ID::CameraMode mode,
	                boost::shared_ptr<Waypoint> wpLocation,
	                const v3& offset,
	                f32 reactionTime,
					f32 maxDistance,
	                f32 stiffness) :
	mMode(mode),
	mMaxRotationVelocity(v3(1.5708f)),
	mMaxSpeed(20.0f * si::meters_per_second),
	mWPLocation(wpLocation),
	mOffset(offset),
	mReactionTime(reactionTime * si::second),
	mMaxDistance(maxDistance * si::meter),
	mTargetOffset(800.0f * si::meter),
	mFullscreen(true),
	mStiffness(stiffness) {}

	ID::CameraMode mMode;

	// free cam parameter
	v3 mMaxRotationVelocity;
	quantity<si::velocity, f32> mMaxSpeed;

	// fixed cam parameter
	boost::shared_ptr<Waypoint> mWPLocation;
	v3 mOffset;

	// chase cam parameters
	quantity<si::time, f32> mReactionTime;
	quantity<si::length, f32> mMaxDistance;

	// target chase parameters
	quantity<si::length, f32> mTargetOffset;

	bool mFullscreen;

	quantity<si::dimensionless, f32> mStiffness;
};

class Camera : public Property::Concept
{
public:
	Camera(GameObject& owner, PluginId pid);
	virtual ~Camera();

private:
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);
	virtual void internalOnEvent(EventIdT action, Property::Value payload, GameHandle module, GameHandle sender);

	virtual void internalOnModuleAttached(GameHandle module);

	void updateOwnLocation(quantity<si::time, f32> timeSinceLastFrame);
	void reset();
	void setMode(ID::CameraMode mode);

	void (Camera::*specificUpdate)(quantity<si::time, f32>);

	void updateFree(quantity<si::time, f32> timeSinceLastFrame);
	void updateFixed(quantity<si::time, f32> timeSinceLastFrame);
	void updateChase(quantity<si::time, f32> timeSinceLastFrame);
	void updateChaseTarget(quantity<si::time, f32> timeSinceLastFrame);

	Location mLastObjLocation;
	Location mOwnLocation;

	v3 mMaxRotationVelocity;
	v3 mRotationInput;
	v3 mLastObjVelocity;

#if 0
	Base::CEaseInOutInterpolation mInterpol;

	quantity<si::velocity, f32> mMaxSpeed;
	quantity<si::velocity, f32> mSpeedInput;
	quantity<si::time, f32> mReactionTime;
	quantity<si::length, f32> mMaxDistance;
	quantity<si::length, f32> mTargetOffset;
	quantity<si::dimensionless, f32> mOffsetFactor;
	quantity<si::dimensionless, s32> mOffsetChange;
	quantity<si::dimensionless, f32> mFactor;
	quantity<si::dimensionless, f32> mStiffness;
#endif

	GameHandle mTarget;
	//boost::shared_ptr<Waypoint> mWaypoint;

	ID::CameraMode mMode;
	v3 mOffset;

	bool mNeedsUpdate;
};

} // namespace BFG

#endif