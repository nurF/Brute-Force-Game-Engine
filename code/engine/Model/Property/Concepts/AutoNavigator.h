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

#ifndef AUTONAVIGATOR_H_
#define AUTONAVIGATOR_H_

#include <boost/units/systems/si/velocity.hpp>
#include <boost/units/systems/si/length.hpp>
#include <boost/units/systems/si/plane_angle.hpp>
#include <boost/units/systems/si/angular_acceleration.hpp>
#include <boost/units/systems/si/time.hpp>
#include <boost/units/quantity.hpp>

#include <Model/Property/Concept.h>

using namespace boost::units;

namespace BFG {

struct Module;

class AutoNavigator : public Property::Concept
{
public:
	AutoNavigator(GameObject& owner, PluginId pid);
	~AutoNavigator();

private:
	typedef std::vector<GameHandle> TargetContainerT;

	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);
	virtual void internalOnEvent(EventIdT action,
	                             Property::Value payload,
	                             GameHandle module,
	                             GameHandle sender);

	void operate();
	void rotate(const qv4& rotation) const;
	void accelerate(quantity<si::velocity, f32> targetSpeed) const;

	void recalculateParameters();

	TargetContainerT mTargets;

	quantity<si::plane_angle, f32>          mOptimalAngle;
	quantity<si::velocity, f32>             mMaxSpeed;
	quantity<si::angular_acceleration, f32> mMaxAngularAcceleration;
	quantity<si::length, f32>               mRadius;
	quantity<si::time,f32>                  mTime;

	v3 mMaxAngularVelocity;
};

} // namespace BFG

#endif