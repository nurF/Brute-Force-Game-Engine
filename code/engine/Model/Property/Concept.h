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

#ifndef PROPERTYCONCEPT_H_
#define PROPERTYCONCEPT_H_

#include <vector>
#include <map>

#include <boost/shared_ptr.hpp>
#include <boost/variant.hpp>

#include <boost/units/systems/si/time.hpp>
#include <boost/units/quantity.hpp>

#include <EventSystem/Core/EventDefs.h>

#include <Core/Types.h>

#include <Model/Defs.h>
#include <Model/Enums.hh>
#include <Model/GameObject.h>
#include <Model/Module.h>
#include <Model/Events/GameObjectEvent_fwd.h>
#include <Model/Property/ConceptId.h>
#include <Model/Property/Value.h>
#include <Model/Property/ValueId.h>

using namespace boost::units;

namespace BFG {

class GameObject;
class Environment;
struct Module;

// This is thought for clients, not for 'class Concept´,
// because concrete concepts are not within namespace Property.
using Property::ValueId;
using Property::PluginId;

namespace Property {

/** \brief
		Base class for Concept 's which
		add functionality to a GameObject.

	Allows communication between derived classes and a GameObject as well as
	the configuration of event forwarding.
*/
class MODEL_API Concept : protected Emitter
{
public:
	//! \brief Container used to return data when calling getValueRange()
	//!        within a Concept or GameObject.
	typedef std::map<
		ConceptId,
		Value
	> ValueAccessContainerT;

	virtual ~Concept();

	void onModuleAttached(boost::shared_ptr<Module>);
	void onModuleDetached(GameHandle);

	void update(quantity<si::time, f32> timeSinceLastFrame);

	void onEvent(EventIdT, Value payload, GameHandle module, GameHandle sender);

	ConceptId concept() const { return mSelf; }
	PluginId  pluginId() const { return mPluginId; }

protected:
	//! Only the SectorFactory and it's helpers are allowed to create properties.
	//! The constructor do not set
	Concept(GameObject& Owner, ConceptId, PluginId);

	//! This function is guaranteed to be called for every Concept
	//! once in sequence per tick.
	//! \see GameObject::update()
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);

	/** \bug
		Do not call setGoValue within this function.
		The application will crash and the reason is, that Concept 's
		need to be informed instantly of changed values. This is done right
		away after setGoValue is done. A ID::GOE_VALUE_UPDATED event will be sent.
		Triggering the same event while catching it -> stack overflow.
	*/
	virtual void internalOnEvent(EventIdT,
	                             Value payload,
	                             GameHandle module,
	                             GameHandle sender);

	virtual void internalOnModuleAttached(GameHandle) {}
	virtual void internalOnModuleDetached(GameHandle) {}

	//! \brief Notifies the owner GameObject that the stated event must
	//!        be forwarded to this Concept.
	//! \see stopDelivery
	void requestEvent(EventIdT);

	//! \brief Stops forwarding of a certain event.
	//! \see requestEvent
	void stopDelivery(EventIdT);

	//! \brief Performs a dependency check.
	//! Mostly called within the constructor of Concept implementations.
	void require(ConceptId) const;

	//! \brief Checks if all necessary Value 's are existing
	//! Initialization is done by Value 's now. Therefore, some of them
	//! are mandatory. Missing PVs may lead to UB or exceptions. So every time
	//! a new Module gets attached to this PC, we'll check here if it has all
	//! PVs which are required for initialization.
	void initvar(ValueId::VarIdT);

	/** \brief Simplified accessor for Property::Value 's.

		This call is the equivalent to:
		\code
		mModules[moduleId]->mValues[ValueId(varId, pluginId())];
		\endcode

		\note The pluginId of this Property::Concept gets used in order to find
		      the Property::Value.

		\param varId    "Variable Id"-Part of the composed ValueId
		\param moduleId Handle of module owning the value.
		\return         A copy of the requested Property::Value
	*/
	template <typename RetT>
	RetT& value(ValueId::VarIdT varId,
	            GameHandle moduleId)
	{
		ModuleMapT::const_iterator moduleIt = mModules.find(moduleId);
		assert(moduleIt != mModules.end());
		
		boost::shared_ptr<Module> module = moduleIt->second;

		Module::ValueStorageT::iterator valueIt = module->mValues.find
		(
			Property::ValueId
			(
				varId,
				pluginId()
			)
		);
		
		// If the Value does not exist yet, add it.
		if (valueIt == module->mValues.end())
		{
			valueIt = module->mValues.insert
			(
				std::make_pair
				(
					Property::ValueId
					(
						varId,
						pluginId()
					),
					RetT()
				)
			).first;
		}
		
		return static_cast<RetT&>(valueIt->second);
	}
	
	/** \brief const version of Concept::value()
	
		\attention The value MUST already exist within the Module.
		           It cannot be added (created) with this function.

		\see RetT& value(ValueId::VarIdT varId, GameHandle moduleId)
	*/
	template <typename RetT>
	const RetT& value(ValueId::VarIdT varId,
	                  GameHandle moduleId) const
	{
		ModuleMapT::const_iterator moduleIt = mModules.find(moduleId);
		assert(moduleIt != mModules.end());
		return moduleIt->second->mValues.find
		(
			Property::ValueId
			(
				varId,
				pluginId()
			)
		)->second;
	}
	
	//! Forwarder to GameObject due to friend limitations
	template <typename RetT>
	const RetT& getGoValue(ValueId::VarIdT varId, PluginId pluginId) const
	{
		return owner().getValue<RetT>(varId, pluginId);
	}

	//! Forwarder to GameObject due to friend limitations
	template <typename ValueT>
	void setGoValue(ValueId::VarIdT varId, PluginId pluginId, const ValueT& value) const
	{
		owner().setValue(varId, pluginId, value);
	}

	//! \brief Accessor for the GameHandle of the root module of
	//!        this GameObject.
	//! \note This handle may not exist within mModules if this Concept is not
	//!       responsible for the root module.
	GameHandle rootModule() const;

	//! \note This is also the handle of the root module.
	GameHandle ownerHandle() const;

	GameObject& owner() const;
	
	//! Forwarder to GameObject::environment()
	const boost::shared_ptr<Environment> environment() const;
	
	const ConceptId mSelf;
	
	const PluginId mPluginId;

	typedef std::map
	<
		GameHandle,
		boost::shared_ptr<Module>
	> ModuleMapT;
	
	ModuleMapT mModules;

private:
	void checkPropertyValues(const boost::shared_ptr<Module>) const;

	GameObject& mOwner;

	std::vector<ValueId::VarIdT> mRequiredInitvars;
};

} // namespace Property
} // namespace BFG

#endif
