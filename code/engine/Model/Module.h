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

#ifndef BFG_MODULE__
#define BFG_MODULE__

#include <Model/Events/GameObjectEvent_fwd.h>
#include <Model/Property/ConceptId.h>
#include <Model/Property/ValueId.h>
#include <Model/Property/Value.h>
#include <Model/Managed.h>

namespace BFG {

/** \brief
		A simple attachable GameObject Module

	GameObject 's are made of one or more Module 's which can be assembled via
	Adapter 's.
*/
struct Module : public Managed
{
	typedef std::vector<Property::ConceptId>             ConceptStorageT;
	typedef std::map<Property::ValueId, Property::Value> ValueStorageT;

	Module(GameHandle handle) :
	Managed(handle, "Module", ID::OT_Module)
	{}

	//! The list of Concept 's which shall have access to this Module.
	ConceptStorageT mPropertyConcepts;

	//! Values which can be read and written by all Concepts which have access
	//! to this Module.
	ValueStorageT mValues;

	virtual void internalUpdate(quantity<si::time, f32> /*timeSinceLastFrame*/)
	{}
};

} // namespace BFG

#endif
