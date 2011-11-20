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

#ifndef DESTROYABLE_H_
#define DESTROYABLE_H_

#include <Model/Property/Concept.h>

namespace BFG {

class Sector;

class Destroyable : public Property::Concept
{
public:
	Destroyable(GameObject& owner, PluginId pid);

private:
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);
	virtual void internalOnEvent(EventIdT action, Property::Value payload, GameHandle module, GameHandle sender);
	
	void updateModule(GameHandle, quantity<si::time, f32> timeSinceLastFrame);
	void updateGui(f32 damage, f32 armor);

	bool isHeavy(GameHandle) const;
	bool shouldRespawn(GameHandle, quantity<si::time, f32> timeSinceLastFrame);

	void destroy(GameHandle, bool respawns);
	void respawn(GameHandle);
	
	const f32 mDamageMultiplier;
	
	std::map<GameHandle, f32> mTimeSinceDestruction;
};

} // namespace BFG

#endif
