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

#ifndef BFG_PROPERTY_PLUGIN_H_
#define BFG_PROPERTY_PLUGIN_H_

#include <vector>
#include <sstream>
#include <stdexcept>

#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/multi_index/identity.hpp>
#include <boost/multi_index/mem_fun.hpp>

#include <Model/Property/ConceptFactory.h>
#include <Model/Property/Serialiser.h>

namespace BFG {
namespace Property {

struct MODEL_API Plugin
{
	Plugin(PluginId id, const std::string& name) :
	mId(id),
	mName(name)
	{}

	virtual ~Plugin() {}

	const std::string& name() const { return mName; }
	PluginId           id() const { return mId; }
	
	virtual boost::shared_ptr<ConceptFactory>
	conceptFactory() const = 0;

	virtual boost::shared_ptr<Serialiser>
	serialiser() const = 0;

	bool
	hasConcept(const ConceptId& concept) const
	{
		return std::find
		(
			mConcepts.begin(),
			mConcepts.end(),
			concept
		) != mConcepts.end();
	}

	friend std::ostream& operator<< (std::ostream& lhs, const Plugin& rhs)
	{
		lhs << rhs.name() << " (Id: " << rhs.id() << ")";
		return lhs;
	}

protected:
	void addConcept(const ConceptId& concept)
	{
		mConcepts.push_back(concept);
	}

private:
	const PluginId    mId;
	const std::string mName;
	
	std::vector<ConceptId> mConcepts;
};

namespace mi = boost::multi_index;
typedef boost::shared_ptr<Plugin> PluginPtr;

typedef boost::multi_index_container
<
	PluginPtr,
	mi::indexed_by <
		mi::ordered_unique <
			mi::const_mem_fun <
				Plugin,
				PluginId,
				&Plugin::id
			>
		>,
		mi::ordered_non_unique <
			mi::const_mem_fun <
				Plugin,
				const std::string&,
				&Plugin::name
			>
		>
	>
> PluginMapT;

template <typename PluginMapT_>
boost::shared_ptr<Concept>
createConcept(const std::string& concept,
              GameObject&        owner, 
              const PluginMapT_& plugins)
{
	typename PluginMapT_::const_iterator it = plugins.begin();
	for (; it != plugins.end(); ++it)
	{
		PluginPtr plugin = *it;
		boost::shared_ptr<Concept> result =
			plugin->conceptFactory()->createConcept(concept, owner);
			
		if (result)
			return result;
	}

	std::stringstream ss;
	ss << "No Property::Plugin found, which is able to "
	   << "create an instance of the concept \"" << concept << "\".";
	throw std::runtime_error(ss.str());

	return boost::shared_ptr<Concept>();
}


template <typename PluginMapT_>
ValueId
symbolToValueId(const std::string& symbol,
                const PluginMapT_& plugins)
{
	typename PluginMapT_::const_iterator it = plugins.begin();
	for (; it != plugins.end(); ++it)
	{
		PluginPtr plugin = *it;
		ValueId::VarIdT var;
		if (plugin->serialiser()->stringToVar(symbol, var))
			return ValueId(var, plugin->id());
	}

	std::stringstream ss;
	ss << "No Property::Plugin found, which is able to "
	   << "serialise \"" << symbol << "\".";
	throw std::runtime_error(ss.str());

	return ValueId();
}

template <typename PluginMapT_>
std::string
valueIdToSymbol(const ValueId& vid,
                const PluginMapT_& plugins)
{
	std::string symbol;
	
	typename PluginMapT_::const_iterator it = plugins.find(vid.mPluginId);
	
	if (it == plugins.end())
	{
		std::stringstream ss;
		ss << "Property::Plugin not found, which is able to "
		   << "serialise \"" << vid << "\".";
		throw std::runtime_error(ss.str());
	}
	
	(*it)->serialiser()->varToString(vid.mVarId, symbol);
	return symbol;

#if 0
	typename PluginMapT_::const_iterator it = plugins.begin();
	
	for (; it != plugins.end(); ++it)
	{
		PluginPtr plugin = *it;
		if (plugin->serialiser()->varToString(vid.mVarId, symbol))
			return symbol;
	}
	
	std::stringstream ss;
	ss << "No Property::Plugin found, which is able to "
	   << "serialise \"" << vid << "\".";
	throw std::runtime_error(ss.str());
		
	return std::string();
#endif
}

template <typename PluginMapT_>
PluginId
pluginIdOfConcept(const ConceptId& concept,
                  const PluginMapT_& plugins)
{
	typename PluginMapT_::const_iterator it = std::find_if
	(
		plugins.begin(),
		plugins.end(),
		boost::bind(&Plugin::hasConcept, _1, concept)
	);

	if (it == plugins.end())
	{
		std::stringstream ss;
		ss << "No Property::Plugin found, which contains a concept of id \""
		   << concept
		   << "\".";
		throw std::runtime_error(ss.str());
	}
	
	PluginPtr plugin = *it;
	return plugin->id();
}

} // namespace Property
} // namespace BFG

#endif
