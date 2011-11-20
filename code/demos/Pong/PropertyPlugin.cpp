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

#include <Pong/PropertyPlugin.h>
#include <Pong/PongFactory.h>

using namespace BFG;

bool PongSerialiser::varToString(Property::ValueId::VarIdT in, std::string& out) const
{
	std::cout << "STUB: PongSerialiser::varToString [" << in << "]" << std::endl;
	return false;
}

bool PongSerialiser::stringToVar(const std::string& in, Property::ValueId::VarIdT& out) const
{
	std::cout << "STUB: PongSerialiser::stringToVar [" << in << "]" << std::endl;
	return false;
}

// ------------------------ PLUGIN -----------------------------

PongPlugin::PongPlugin(Property::PluginId pluginId) :
Plugin(pluginId, "Pong Properties")
{
	addConcept("BarControl");
	addConcept("BallControl");
}

boost::shared_ptr<Property::ConceptFactory>
PongPlugin::conceptFactory() const
{
	boost::shared_ptr<Property::ConceptFactory> f(new PongConceptFactory(id()));
	return f;
}

boost::shared_ptr<Property::Serialiser>
PongPlugin::serialiser() const
{
	boost::shared_ptr<Property::Serialiser> s(new PongSerialiser);
	return s;
}