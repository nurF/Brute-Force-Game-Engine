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

#ifndef GAMEOBJECT_H__
#define GAMEOBJECT_H__

#include <boost/noncopyable.hpp>
#include <boost/unordered_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/weak_ptr.hpp>

#include <Core/Location.h>

#include <EventSystem/Emitter.h>

#include <Model/Adapter.h>
#include <Model/Managed.h>
#include <Model/Module.h>
#include <Model/Property/ConceptId.h>
#include <Model/Property/Value.h>
#include <Model/Property/Plugin.h>

class EventLoop;

namespace BFG {

class Environment;
struct Waypoint;

namespace Property
{
	class Concept;
}

using Property::ValueId;
using Property::PluginId;

/** \brief
		All game related objects, such as Ships, Gates, Rockets etc. are
		GameObject 's with different Property::Concept 's.

	GameObject currently serves as a bigger container and event delegater.
	It is also responsible for synchronizing the Model part (ODE) and the
	View part (OGRE).
	
	\see updateView()
*/
class MODEL_API GameObject : public Managed,
                             boost::noncopyable,
                             Emitter
{
public:
	typedef boost::shared_ptr<Managed> VertexData;
	typedef Adapter                    EdgeData;

    typedef boost::adjacency_list 
    < 
        boost::vecS, 
        boost::vecS, 
        boost::bidirectionalS,
        VertexData,
        EdgeData
    > ModuleTreeT;

	typedef ModuleTreeT::vertex_descriptor VD;
	typedef ModuleTreeT::edge_descriptor ED;

	typedef boost::shared_ptr<Managed> ChildT;

	         GameObject(EventLoop* loop,
	                    const GameHandle handle,
	                    const std::string& publicName,
	                    const Property::PluginMapT& propertyPlugins,
	                    boost::shared_ptr<Environment> environment);

	virtual ~GameObject();

	/** \brief
		Attaches a Module to another Module in this GameObject.

		Please do NOT construct ring-like-connected structures. This may cause
		a stack overflow in detachModule() due to recursion.
		
		\param[in] managed
			The Module to be attached.

		\param[in] adapters
			List of Adapter 's of the new Module.
			
		\param[in] childAdapterID
			Identifier of the Adapter on the new Module to which the parent
			shall get connected.

		\param[in] parent
			Handle of Parent to which the module shall get connected.
			Pass NULL_HANDLE for the root Module.
			
		\param[in] parentAdapterID
			Identifier of the Adapter of the parent Module to which the new
			Module shall get connected.
	*/
	void attachModule(ChildT managed,
	                  const std::vector<Adapter>& adapters,
	                  const u32 childAdapterID,
	                  const GameHandle parent,
	                  const u32 parentAdapterID);


	//! We'll store it in a much simpler container if this module is not
	//! physical.
	void attachModule(boost::shared_ptr<Module> module);

	//! \brief
	//!		Destroys the Module with the defined handle and all its children.
	void detachModule(GameHandle handle);

	void EventHandler(GameObjectEvent* goe);

	bool satisfiesRequirement(Property::ConceptId concept) const;

	void debugOutput(std::string& output) const;

	//! \see GameObject::setValue()
	template <typename RetT>
	const RetT&
	getValue(ValueId::VarIdT varId, PluginId pluginId) const
	{
		ValueId vid(varId, pluginId);
		Module::ValueStorageT::const_iterator it;
		it = mValues.find(vid);
		if (it == mValues.end())
		{
			std::stringstream ss;
			ss << "GameObject::getValue: Value "
			   << valueIdToSymbol(vid, plugins())
			   << " not available!\n\n";
			std::string dbg;
			debugOutput(dbg);
			ss << dbg;
			throw std::runtime_error(ss.str());
		}
		return it->second;
	}
	
	const boost::shared_ptr<Environment> environment() const;
	
	//! \return A list of all adapters of the root module.
	const std::vector<Adapter>& rootAdapters() const;

	const Property::PluginMapT& plugins() const { return mPropertyPlugins; }
	
	bool docked() const { return mDocked; }

	void activate() { mActivated = true; }

private:
	friend class Property::Concept;

	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);

	//! PropertyConcepts need events too. They register their needs indirectly
	//! through their base class which then calls this function.
	//! \see Property::Concept::requestEvent
	void registerNeedForEvent(boost::shared_ptr<Property::Concept>, EventIdT);

	//! \see GameObject::registerNeedForEvent
	void unregisterNeedForEvent(boost::shared_ptr<Property::Concept>, EventIdT);

	//! Helper function to distribute an event amongst all attached
	//! PropertyConcepts of this GameObject.
	void distributeEvent(EventIdT action,
	                     const Property::Value& payload,
	                     GameHandle receivingModule,
	                     GameHandle sender);

	/** \brief Sets a Property::Value for the whole GameObject
		
		Concepts may call this function to set Property::Value 's
		which are seen in a context for the whole GameObject and not for one
		single Module only.
		
		\note
			As with Module 's, all Concept 's of this GameObject (even from
			different Property::Plugin 's) have rw-access to these values.
		
		External Module 's have read-only access to these values by using
		the const version of this function.
	*/
	void setValue(Property::ValueId::VarIdT varId,
	              Property::PluginId pluginId,
	              const Property::Value& value);

	void setRequirement(Property::ConceptId self, Property::ConceptId other);

	void deleteVertex(VD vd);

	VD   findVertex(GameHandle handle) const;
	
	void vectorToModuleFromRoot(VD module, v3& vecResult, qv4& oriResult) const;

	void notifyPropertyConcepts(boost::shared_ptr<Module>);
	
	void connectChildToParent(VD parentVd,
                              u32 parentAdapterID,
                              VD childVd,
                              u32 childAdapterID);
	
	void connectOtherGameObject(boost::shared_ptr<GameObject> other,
	                            const v3& position,
	                            const qv4& orientation);
	
	void disconnectOtherGameObject(boost::shared_ptr<GameObject> other,
	                               const v3& position,
	                               const qv4& orientation);

	bool hasModuleWithHandle(GameHandle) const;

	bool hasSameIndex(ModuleTreeT::edge_descriptor e,
	                  u32 Index,
	                  Adapter& oldAdapter) const;

	/**
		Property::Concept 's may depend on each other. That's why, a correct
		update order is crucial. The order resembles a tree structure whose
		deepest nodes get updated first.
		
		Every time a new Concept gets instantiated within this GameObject, this
		function gets called and the order will be stored into a container,
		which is then accessed by internalUpdate().
		
		Assuming that we'd have the following Concept dependency hierarchy:
		
		\verbatim
		                  0
		                  |
		         8        1
		       / | \      |
		      7 12  9     2

		\endverbatim

		The order would be: 2, 1, 0, 3, 4, 5, 6, 7, 12, 9, 8, 10, 11.
	*/
	void rebuildConceptUpdateOrder();

	//! Container type for storage of Property::Concept 's
	typedef boost::unordered_map
	<
		Property::ConceptId,
		boost::shared_ptr<Property::Concept>
	> ConceptContainerT;

	//! Container type for storage of event requests from Property::Concept 's.
	typedef boost::unordered_multimap
	<
		EventIdT,
		boost::shared_ptr<Property::Concept>
	> EventDemandContainerT;

	typedef std::map
	<
		GameHandle,
		boost::shared_ptr<Module>
	> VirtualModuleMapT;

	//! 'First' is Self, 'Second' is Other
	typedef std::multimap
	<
		Property::ConceptId,
		Property::ConceptId
	> ConceptDependenciesT;

	typedef std::vector
	<
		boost::weak_ptr<Property::Concept>
	> UpdateOrderContainerT;
	
	boost::shared_ptr<Environment> mEnvironment;

	//! Concept dependencies set up via setRequirement()
	ConceptDependenciesT      mConceptDependencies;

	//! Correct update order generated by rebuildConceptUpdateOrder()
	UpdateOrderContainerT     mConceptUpdateOrder;

	ModuleTreeT               mModules;
	VirtualModuleMapT         mVirtualModules;
	
	const Property::PluginMapT& mPropertyPlugins;
	
	Module::ValueStorageT     mValues;
	ConceptContainerT         mConcepts;
	EventDemandContainerT     mEventDemands;

	//! Cache for hasModuleWithHandle()
	std::vector<GameHandle> mModuleHandles;

	VD mDummy;

	bool mDocked;
	bool mActivated;
	
	std::vector<Adapter> mRootAdapters;
};

std::ostream& operator << (std::ostream& lhs, const GameObject& rhs);

} // namespace BFG

#endif
