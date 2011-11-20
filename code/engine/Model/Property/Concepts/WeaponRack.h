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

#ifndef WEAPONRACK_H_
#define WEAPONRACK_H_

#include <Model/Property/Concept.h>

namespace BFG {

namespace Loader {
	class SectorFactory;
}

class WeaponRack : public Property::Concept
{
public:
	const static s32 ROCKET_AMMO_START_AMOUNT_FOR_TESTING = 100;
	
	WeaponRack(GameObject& owner, PluginId pid);

private:
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);
	virtual void internalOnEvent(EventIdT action, Property::Value payload, GameHandle module, GameHandle sender);

	void fireRocket();
	void fireLaser();

	void updateGuiAmmo() const;
	
	void calculateVelocity(f32 startImpulse,
	                       v3& newVelocity,
	                       bool addShipVelocity) const;

	void calculateSpawnLocation(Location& spawnLocation,
	                            f32 spawnDistance) const;

	s32 mRocketAmmo;

	GameHandle mTarget;
};

} // namespace BFG

#endif