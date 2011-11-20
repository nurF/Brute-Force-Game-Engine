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

#ifndef THRUSTCONTROL_H_
#define THRUSTCONTROL_H_

#include <boost/shared_ptr.hpp>

#include <boost/units/systems/si/acceleration.hpp>
#include <boost/units/systems/si/angular_acceleration.hpp>
#include <boost/units/systems/si/torque.hpp>
#include <boost/units/systems/si/force.hpp>
#include <boost/units/systems/si/moment_of_inertia.hpp>
#include <boost/units/systems/si/mass.hpp>
#include <boost/units/systems/si/power.hpp>
#include <boost/units/systems/si/velocity.hpp>
#include <boost/units/quantity.hpp>

#include <Core/ExternalTypes_fwd.h>
#include <Model/Property/Concept.h>

using namespace boost::units;

namespace BFG {

class ThrustControl : public Property::Concept
{
public:
	ThrustControl(GameObject& owner, PluginId pid);
	~ThrustControl();
private:

	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);
	virtual void internalOnEvent(EventIdT action, Property::Value payload, GameHandle module, GameHandle sender);
	
	v3 calculateForce(const v3& targetSpeed,
	                  const v3& currentSpeed,
	                  quantity<si::force, f32> forceLimit,
	                  quantity<si::time, f32> timestep);

	v3 calculateTorque(const v3& targetSpeed,
	                   const v3& currentSpeed,
	                   const m3x3& inertia,
	                   quantity<si::force, f32> forceLimit,
	                   quantity<si::time, f32> timestep);

	void magicStop();
	
	void calculateEngineAttributes();

	quantity<si::mass, f32>                 mTotalWeight;
	quantity<si::force, f32>                mTotalEngineForce;
	quantity<si::force, f32>                mTotalManeuverForce;

	quantity<si::velocity, f32>             mMaxSpeed;
	v3                                      mMaxRotationSpeed;

	quantity<si::acceleration, f32>         mMaxAcceleration;
	quantity<si::angular_acceleration, f32> mMaxAngularAcceleration;

	v3                                      mTargetSpeed;
	v3                                      mTargetRotationSpeed;
	
	v3                                      mRelativeEnginePosition;
};

} // namespace BFG

#endif