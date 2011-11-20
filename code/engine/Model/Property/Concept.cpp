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

#include <Model/Property/Concept.h>

#include <Base/CLogger.h>
#include <Base/Cpp.h>

#include <Model/GameObject.h>
#include <Model/Module.h>

namespace BFG {
namespace Property {

Concept::Concept(GameObject& Owner, ConceptId pc, PluginId pid) :
Emitter(Owner.loop()),
mSelf(pc),
mPluginId(pid),
mOwner(Owner)
{}

Concept::~Concept()
{
	//! \todo Delete me
	dbglog << "Concept "
	       << mSelf
	       << " destroyed (owner: "
	       << ownerHandle()
	       << ")";
}

void Concept::onModuleAttached(boost::shared_ptr<Module> module)
{
	checkPropertyValues(module);
	mModules[module->getHandle()] = module;
	internalOnModuleAttached(module->getHandle());
}

void Concept::onModuleDetached(GameHandle module)
{
	mModules.erase(module);
	internalOnModuleDetached(module);
}

void Concept::update(quantity<si::time, f32> timeSinceLastFrame)
{
	this->internalUpdate(timeSinceLastFrame);
}

void Concept::onEvent(EventIdT action,
                      Value payload,
                      GameHandle module,
                      GameHandle sender)
{
	this->internalOnEvent(action, payload, module, sender);
}

void Concept::internalUpdate(quantity<si::time, f32> /*timeSinceLastFrame*/)
{
}

void Concept::internalOnEvent(EventIdT,
                              Value /*payload*/,
                              GameHandle /*module*/,
                              GameHandle /*sender*/)
{
}

void Concept::requestEvent(EventIdT action)
{
	boost::shared_ptr<Concept> This(this, null_deleter());
	mOwner.registerNeedForEvent(This, action);
}

void Concept::stopDelivery(EventIdT action)
{
	boost::shared_ptr<Concept> This(this, null_deleter());
	mOwner.unregisterNeedForEvent(This, action);
}

void Concept::require(ConceptId pc) const
{
	if (! mOwner.satisfiesRequirement(pc))
		warnlog << "Object " << ownerHandle()
		        << " does not satisfy the requirement Concept:" << pc
		        << " which is needed by Concept:" << mSelf;
	
	mOwner.setRequirement(mSelf, pc);
}

void Concept::initvar(ValueId::VarIdT id)
{
	mRequiredInitvars.push_back(id);	
}

GameHandle Concept::rootModule() const
{
	// At the moment, the GameHandle of the GameObject is also
	// the handle of the root module.
	return ownerHandle();
}

GameHandle Concept::ownerHandle() const
{
	return mOwner.getHandle();
}

GameObject& Concept::owner() const
{
	return mOwner;
}

const boost::shared_ptr<Environment> Concept::environment() const
{
	return mOwner.environment();		
}

void Concept::checkPropertyValues(const boost::shared_ptr<Module> module) const
{
	std::vector<ValueId::VarIdT> missing;
	
	for (size_t i=0; i<mRequiredInitvars.size(); ++i)
	{
		bool found = module->mValues.find
		(
			ValueId
			(
				mRequiredInitvars[i],
				mPluginId
			)
		) != module->mValues.end();

		if (! found)
		{
			missing.push_back(mRequiredInitvars[i]);
		}
	}

	if (missing.empty())
		return;

	std::stringstream ss;
	ss << "Tried to attach a Module which is still missing some Values"
	      " required for initialization of Concept \""
	   << concept()
	   << "\": \n";
	
	for (size_t i=0; i<missing.size(); ++i)
	{
		const PluginPtr p = *owner().plugins().find(pluginId());
		std::string varId;
		if (p->serialiser()->varToString(missing[i], varId))
		{
			ss << "- " << varId << "\n";
		}
		else
		{
			ss << "- " << "VarId: " << missing[i] << "\n";
		}
	}
		
	throw std::runtime_error(ss.str());
}

} // namespace Property
} // namespace BFG
