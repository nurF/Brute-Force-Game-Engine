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

#ifndef BFG_PROPERTYCONCEPT_FACTORY_H__
#define BFG_PROPERTYCONCEPT_FACTORY_H__

#include <boost/shared_ptr.hpp>
#include <Core/Types.h>
#include <Model/Property/ValueId.h>
#include <Model/Property/ConceptId.h>
#include <Model/Enums.hh>
#include <Model/Defs.h>

#define PROPERTYCONCEPT_BUILD_LIST_BEGIN                    \
	if (pc.empty()) throw std::logic_error                  \
		("[virtual] ConceptFactory: No factory can create a nameless Concept"); \
	if (false)                                              \
	{                                                       \
	}

#define PROPERTYCONCEPT_CASE(T)                             \
    else if (#T == pc)                                      \
    {                                                       \
        boost::shared_ptr<T> p(new T(go, pluginId()));      \
        return p;                                           \
    }

#define PROPERTYCONCEPT_BUILD_LIST_END                      \
	else                                                    \
	{                                                       \
		return boost::shared_ptr<BFG::Property::Concept>(); \
	}

namespace BFG {

	class GameObject;

namespace Property {

	class Concept;

class MODEL_API ConceptFactory
{
public:
	ConceptFactory(PluginId pid) :
	mPluginId(pid)
	{}

	virtual ~ConceptFactory() {}

	//! \return Either the fully constructed Concept or a default initialised
	//!         shared_ptr (NULL), if the factory is not responsible for the
	//!         creation of the desired concept.
	virtual boost::shared_ptr<Concept>
	createConcept(ConceptId pc, GameObject& go) = 0;
	
protected:
	PluginId pluginId() const { return mPluginId; }
	
private:
	const PluginId mPluginId;
};

} // namespace Property
} // namespace BFG

#endif