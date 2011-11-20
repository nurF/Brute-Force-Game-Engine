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


#ifndef LOADER_TAGS_H__
#define LOADER_TAGS_H__

namespace BFG
{
namespace Loader
{


namespace Sign
{

typedef const char SignT;

SignT interrupt = ':';

} // Sign

namespace Prefix
{

typedef const std::string PrefixT;

PrefixT object = "object";
PrefixT position = "position";
} // Prefix

namespace Tag
{

typedef const std::string TagT;

//! General (frequently used)
TagT name = "name";
TagT publicName = "publicName";
TagT filename = "filename";
TagT type = "type";
TagT path = "path";
TagT position = "position";
TagT orientation = "orientation";
TagT linear_velocity = "linear_velocity";
TagT angular_velocity = "angular_velocity";
TagT direction = "direction";
TagT offset = "offset";
TagT weight = "weight";
TagT radius = "radius";
TagT maxSpeed = "maxSpeed";
TagT id = "id";
TagT value = "value";
TagT connection = "connection";

//! Concepts
TagT properties = "properties";

//! Module
TagT mesh = "mesh";
TagT adapters = "adapters";
TagT concepts = "concepts";
TagT collision = "collision";
TagT visible = "visible";
TagT density = "density";

//! Conditions 
TagT countdown = "countdown";
TagT laps = "laps";
TagT finish = "finish";
TagT timeout = "timeout";

//! Camera
TagT fullscreen = "fullscreen";
TagT reactionTime = "reactionTime";
TagT maxDistance = "maxDistance";
TagT maxRotationVelocity = "maxRotationVelocity";
TagT stiffness = "stiffness";

//! SectorProperties
TagT playerObject = "playerObject";


//! PropertieConcepts

//! Energy Properties and related values (Accumulator, EnergyCell, Reactor)
TagT transferPower = "transferPower";
TagT capacity = "capacity";
TagT energyNeed = "energyNeed";
TagT shutdownPriority = "shutdownPriority";
TagT emission = "emission";
TagT standbyNeed = "standbyNeed";

//! Auto Navigator
TagT optimalAngleRad = "optimalAngleRad";
TagT maxAngularVelocity = "maxAngularVelocity";

//! Engine
TagT maxRotationSpeedRad = "maxRotationSpeedRad";
TagT engineForce = "engineForce";
TagT maneuverForce = "maneuverForce";
TagT driftDamp = "driftDamp";
TagT rollDamp = "rollDamp";

//! Hull
TagT inertia = "inertia";
TagT armor = "armor";

//! Destroyable
TagT respawn = "respawn";
TagT delay = "delay";

//! InertiaModulator
TagT powerCoefficient = "powerCoefficient";
TagT minInertia = "minInertia";
TagT maxInertia = "maxInertia";

} // Tag

namespace XmlTags
{

typedef const std::string TagT;

TagT ObjectTypes = "ObjectTypes";
TagT ObjectType = "ObjectType";
TagT Sector = "Sector";
TagT ObjectList = "ObjectList";
TagT Object = "Object";
TagT PathList = "PathList";
TagT Path = "Path";
TagT Conditions = "Conditions";
TagT View = "View";
TagT Camera = "Camera";
TagT Light = "Light";
TagT Skybox = "Skybox";
TagT SectorProperties = "SectorProperties";
TagT PlayList = "PlayList";
TagT Song = "Song";
TagT ObjectConfig = "ObjectConfig";
TagT ValueConfig = "ValueConfig";
TagT ConceptConfig = "ConceptConfig";
TagT AdapterConfig = "AdapterConfig";

TagT ObjectConfigs = "ObjectConfigs";
TagT ValueConfigs = "ValueConfigs";
TagT ConceptConfigs = "ConceptConfigs";
TagT AdapterConfigs = "AdapterConfigs";

}

} // Loader
} // namespace BFG

#endif
