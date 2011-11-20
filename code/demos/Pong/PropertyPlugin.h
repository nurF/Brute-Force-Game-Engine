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

#ifndef PONG_PROPERTY_PLUGIN_H
#define PONG_PROPERTY_PLUGIN_H

#include <Model/Property/Plugin.h>

struct PongSerialiser : BFG::Property::Serialiser
{
	virtual ~PongSerialiser() {}

	//! \brief VarId to String serialisation
	//! \param[in] in VarId which shall be serialised to a String
	//! \param[out] out Result
	//! \return true if serialisation was successful, false otherwise
	virtual bool varToString(BFG::Property::ValueId::VarIdT in,
	                         std::string& out) const;

	//! \brief String to VarId serialisation
	//! \param[in] in String which shall be serialised to a VarId
	//! \param[out] out Result
	//! \return true if serialisation was successful, false otherwise
	virtual bool stringToVar(const std::string& in,
	                         BFG::Property::ValueId::VarIdT& out) const;
};

// ------------------------ SPACE PLUGIN -----------------------------

struct PongPlugin : BFG::Property::Plugin
{
	PongPlugin(BFG::Property::PluginId pluginId);
	
	virtual ~PongPlugin() {}

	virtual boost::shared_ptr<BFG::Property::ConceptFactory>
	conceptFactory() const;

	virtual boost::shared_ptr<BFG::Property::Serialiser>
	serialiser() const;
};

#endif