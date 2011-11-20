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

#include <Model/Property/SpacePlugin.h>
#include <Model/Property/SpaceConceptFactory.h>

namespace BFG {

static void ensurePrefix(std::string& identifier, const std::string& prefix)
{
	if (identifier.compare(0, prefix.size(), prefix) != 0)
		identifier = prefix + identifier;
}

bool SpaceSerialiser::varToString(ValueId::VarIdT in, std::string& out) const
{
	try
	{
		out = ID::asStr(static_cast<ID::PropertyVar>(in));
		return true;
	}
	catch (std::out_of_range&)
	{
		return false;
	}		
}

bool SpaceSerialiser::stringToVar(const std::string& in, ValueId::VarIdT& out) const
{
	try
	{
		std::string cmp = in;
		ensurePrefix(cmp, "PV_");
		out = ID::asPropertyVar(cmp);
		return true;
	}
	catch (std::out_of_range&)
	{
		return false;
	}		
}

// ------------------------ SPACE PLUGIN -----------------------------

SpacePlugin::SpacePlugin(PluginId pluginId) :
Plugin(pluginId, "Engine Properties")
{
	addConcept("AutoNavigator");
	addConcept("Camera");
	addConcept("Destroyable");
	addConcept("Physical");
	addConcept("SelfDestruction");
	addConcept("ThrustControl");
	addConcept("Triggers");
	addConcept("WeaponRack");
}

boost::shared_ptr<Property::ConceptFactory>
SpacePlugin::conceptFactory() const
{
	boost::shared_ptr<Property::ConceptFactory> f(new SpaceConceptFactory(id()));
	return f;
}

boost::shared_ptr<Property::Serialiser>
SpacePlugin::serialiser() const
{
	boost::shared_ptr<Property::Serialiser> s(new SpaceSerialiser);
	return s;
}

} // namespace BFG
