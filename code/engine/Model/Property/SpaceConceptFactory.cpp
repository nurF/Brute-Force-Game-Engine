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

#include <Model/Property/SpaceConceptFactory.h>

#include <stdexcept>
#include <Model/Property/Concepts/AutoNavigator.h>
#include <Model/Property/Concepts/Camera.h>
#include <Model/Property/Concepts/Destroyable.h>
#include <Model/Property/Concepts/Physical.h>
#include <Model/Property/Concepts/SelfDestruction.h>
#include <Model/Property/Concepts/ThrustControl.h>
#include <Model/Property/Concepts/Triggers.h>
#include <Model/Property/Concepts/WeaponRack.h>

namespace BFG {

SpaceConceptFactory::SpaceConceptFactory(PluginId pid) :
ConceptFactory(pid)
{}

boost::shared_ptr<Property::Concept>
SpaceConceptFactory::createConcept(Property::ConceptId pc,
                                   GameObject& go)
{
	PROPERTYCONCEPT_BUILD_LIST_BEGIN
	PROPERTYCONCEPT_CASE(AutoNavigator)
	PROPERTYCONCEPT_CASE(Camera)
	PROPERTYCONCEPT_CASE(Destroyable)
	PROPERTYCONCEPT_CASE(Physical)
	PROPERTYCONCEPT_CASE(SelfDestruction)
	PROPERTYCONCEPT_CASE(ThrustControl)
	PROPERTYCONCEPT_CASE(Triggers)
	PROPERTYCONCEPT_CASE(WeaponRack)
	PROPERTYCONCEPT_BUILD_LIST_END
}

} // namespace BFG