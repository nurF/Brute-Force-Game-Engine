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

#ifndef PHYSICAL_H_
#define PHYSICAL_H_

#include <Model/Property/Concept.h>
#include <Physics/Event_fwd.h>

using namespace boost::units;

namespace BFG {

class Physical : public Property::Concept
{
public:
	Physical(GameObject& owner, PluginId pid);
	~Physical();

private:
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);

	virtual void onPhysicsEvent(Physics::Event*);

	void onFullSync(const Physics::FullSyncData& fsd);

	void onPosition(const v3& newPosition);
	void onOrientation(const qv4& newOrientation);
	void onVelocity(const Physics::VelocityComposite& vel);
	void onRotationVelocity(const Physics::VelocityComposite& vel);
	void onTotalMass(const f32 totalMass);
	void onInertia(const m3x3& inertia);
	
	void synchronizeView() const;

	std::vector<ID::PhysicsAction> mPhysicsActions;
};

} // namespace BFG

#endif