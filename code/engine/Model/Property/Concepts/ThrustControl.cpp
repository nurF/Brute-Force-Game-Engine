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

#include <Model/Property/Concepts/ThrustControl.h>

#include <boost/geometry/arithmetic/dot_product.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/plane_angle.hpp>

#include <Core/ExternalTypes.h>
#include <Core/Math.h>

#include <Base/CLogger.h>

#include <Model/Module.h>
#include <Model/GameObject.h>

#include <Physics/Event.h>

BOOST_UNITS_STATIC_CONSTANT
(
	kilogram_meter_squared_per_radian_squared,
	si::moment_of_inertia
);

BOOST_UNITS_STATIC_CONSTANT
(
	radian_per_second_squared,
	si::angular_acceleration
);

namespace BFG {

ThrustControl::ThrustControl(GameObject& owner, PluginId pid) :
Property::Concept(owner, "ThrustControl", pid),
mTargetSpeed(v3::ZERO),
mTargetRotationSpeed(v3::ZERO)
{
	require("Physical");
	
	requestEvent(ID::GOE_VALUE_UPDATED);
	requestEvent(ID::GOE_CONTROL_PITCH);
	requestEvent(ID::GOE_CONTROL_YAW);
	requestEvent(ID::GOE_CONTROL_ROLL);
	requestEvent(ID::GOE_CONTROL_THRUST);
	requestEvent(ID::GOE_CONTROL_MAGIC_STOP);
	
	requestEvent(ID::PE_FULL_SYNC);
	
	initvar(ID::PV_MaxSpeed);
	initvar(ID::PV_EngineForce);
	initvar(ID::PV_ManeuverForce);
	initvar(ID::PV_MaxRotationSpeed);
	initvar(ID::PV_MaxSpeed);
}

ThrustControl::~ThrustControl()
{
}

void ThrustControl::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	assert(mModules.begin()->first == rootModule() &&
		"ThrustControl: Must be in charge of the root module!");

	v3 force = v3::ZERO;
	v3 torque = v3::ZERO;

	const v3& currentSpeed = getGoValue<v3>(ID::PV_RelativeVelocity, pluginId());

	emit<GameObjectEvent>(ID::GOE_VELOCITY, currentSpeed, 0, ownerHandle());

	force = calculateForce
	(
		mTargetSpeed,
		currentSpeed,
		mTotalEngineForce,
		timeSinceLastFrame
	);

	const v3& currentRotSpeed = getGoValue<v3>(ID::PV_RelativeRotationVelocity, pluginId());

	const m3x3& inertia = getGoValue<m3x3>(ID::PV_Inertia, pluginId());

	torque = calculateTorque
	(
		mTargetRotationSpeed,
		currentRotSpeed,
		inertia,
		mTotalManeuverForce,
		timeSinceLastFrame
	);
	
	emit<Physics::Event>(ID::PE_APPLY_FORCE, force, ownerHandle());
	emit<Physics::Event>(ID::PE_APPLY_TORQUE, torque, ownerHandle());
}

void ThrustControl::internalOnEvent(EventIdT action,
                                    Property::Value payload,
                                    GameHandle module,
                                    GameHandle sender)
{
	switch(action)
	{
	case ID::GOE_CONTROL_PITCH:
	{
		f32 targetSpeed(payload * mMaxRotationSpeed.x);
		mTargetRotationSpeed.x = clamp(targetSpeed,
		                               -mMaxRotationSpeed.x,
		                               mMaxRotationSpeed.x);
		break;
	}

	case ID::GOE_CONTROL_YAW:
	{
		f32 targetSpeed(payload * mMaxRotationSpeed.y);
		mTargetRotationSpeed.y = clamp(targetSpeed,
		                               -mMaxRotationSpeed.y,
		                               mMaxRotationSpeed.y);
		break;
	}

	case ID::GOE_CONTROL_ROLL:
	{
		f32 targetSpeed(payload * mMaxRotationSpeed.z);
		mTargetRotationSpeed.z = clamp(targetSpeed,
		                               -mMaxRotationSpeed.z,
		                               mMaxRotationSpeed.z);
		break;
	}

	case ID::GOE_CONTROL_THRUST:
	{
		typedef quantity<si::velocity, f32> VelocityT;
		typedef quantity<si::dimensionless, f32> DimensionlessT;
		
		f32 factor = payload;
		VelocityT targetSpeed = factor * mMaxSpeed;

		mTargetSpeed = v3(0, 0, clamp(targetSpeed.value(),
		                              -mMaxSpeed.value(),
		                              mMaxSpeed.value()));

		emit<GameObjectEvent>
		(
			ID::GOE_ENGINEOUTPUT,
			(f32) (mTargetSpeed.z / mMaxSpeed.value()),
			0,
			ownerHandle()
		);
		break;
	}

	case ID::GOE_CONTROL_MAGIC_STOP:
		magicStop();
		break;

	case ID::GOE_VALUE_UPDATED:
	{
		Property::ValueId valueId = payload;
		if (valueId.mPluginId == pluginId())
			if (valueId.mVarId == ID::PV_Mass ||
			    valueId.mVarId == ID::PV_Inertia)
				calculateEngineAttributes();
		break;
	}

	default:
		throw std::logic_error("wtf?");
	}
}

v3 ThrustControl::calculateForce(const v3& targetSpeed,
								 const v3& currentSpeed,
								 quantity<si::force, f32> forceLimit,
								 quantity<si::time, f32> timestep)
{
	// Calculate the force we need to reach the target speed
	v3 acceleration((targetSpeed - currentSpeed) / timestep.value());

	v3 force = acceleration * mTotalWeight.value();
	
	force = clamp
	(
		force,
		v3(- forceLimit.value()),
	    v3(forceLimit.value())
	);
	
	return force;
}

v3 ThrustControl::calculateTorque(const v3& targetSpeed,
								  const v3& currentSpeed,
								  const m3x3& tensor,
								  quantity<si::force, f32> forceLimit,
								  quantity<si::time, f32> timestep)
{
	// Calculate the force we need to reach the target speed
	v3 omega = targetSpeed - currentSpeed;
	if (length(omega) < EPSILON_F)
		return omega;

	v3 alpha = omega / timestep.value();

	f32 inertia = 0.0f;
	for (int i = 0; i < 3; ++i)
	{
		for (int j = 0; j < 3; ++j)
		{
			inertia += tensor[i][j] * omega.ptr()[i] * omega.ptr()[j];
		}
	}

	inertia *= 1 / boost::geometry::dot_product(omega, omega);

	v3 torque = alpha * inertia;

// 	v3 acceleration((targetSpeed - currentSpeed) / timestep.value());
// 
// 	v3 torque = inertia * acceleration;
// 
	torque = clamp
	(
		torque,
		v3(- forceLimit.value()),
	    v3(forceLimit.value())
	);

	return torque;
}

void ThrustControl::magicStop()
{
	mTargetRotationSpeed = v3::ZERO;
	mTargetSpeed = v3::ZERO;
	
	emit<Physics::Event>(ID::PE_MAGIC_STOP, 0, ownerHandle());
}

void ThrustControl::calculateEngineAttributes()
{
	//! \todo add multiple engine support. More than one engine causes vibrations
	//!       of the gameObject due to overridden values.
	assert(mModules.size() == 1 && 
		"ThrustControl: only one Engine allowed at the moment!");

	// GameObject values
	mTotalWeight = getGoValue<f32>(ID::PV_Mass, pluginId()) * si::kilogram;
	mTotalEngineForce = 0.0f * si::newton;
	mTotalManeuverForce = 0.0f * si::newton;
	mMaxSpeed = 0.0f * si::metres_per_second;
	mMaxRotationSpeed = v3::ZERO;
	mMaxAcceleration = 0.0f * si::meter_per_second_squared;
	mMaxAngularAcceleration = 0.0f * radian_per_second_squared;

	// Module values
	ModuleMapT::iterator it = mModules.begin();
	for (; it != mModules.end(); ++it)
	{
		GameHandle module = it->first;
	
		mTotalEngineForce += value<f32>(ID::PV_EngineForce, module) * si::newton;
		mTotalManeuverForce += value<f32>(ID::PV_ManeuverForce, module) * si::newton;

		quantity<si::velocity, f32> maxSpeed =
			si::meters_per_second * value<f32>(ID::PV_MaxSpeed, module);

		if (maxSpeed > mMaxSpeed)
		{
			mMaxSpeed = maxSpeed;
		}

		const v3& maxRotationSpeed = value<v3>(ID::PV_MaxRotationSpeed, module);

		ceil(mMaxRotationSpeed, maxRotationSpeed);
	}
	
	mRelativeEnginePosition = value<Location>(ID::PV_Location, rootModule()).position;

	if (mTotalWeight > (0.0f * si::kilogram))
	{
		// Rad/s² = F / m
		mMaxAcceleration = mTotalEngineForce / mTotalWeight;
		
		// Rad/s² = F / m * |pos|
		quantity<si::length, f32> l = length(mRelativeEnginePosition) * si::meter;
		quantity<si::acceleration, f32> a = mTotalManeuverForce / mTotalWeight;
		quantity<si::plane_angle, f32> r = 1.0f * si::radian;

		if(l.value() < EPSILON_F)
		{
			dbglog << "ThrustControl::calculateEngineAttributes: An engine should not "
			          "be at (0,0,0)! Using default (0,0,-1).";
			mRelativeEnginePosition = v3(0.0f, 0.0f, -1.0f);
			l = 1.0f * si::meter;
		}
		mMaxAngularAcceleration = a * r / l;
	}
	
	setGoValue(ID::PV_MaxSpeed, pluginId(), mMaxSpeed.value());
	setGoValue(ID::PV_MaxAngularAcceleration, pluginId(), mMaxAngularAcceleration.value());
}


} // namespace BFG