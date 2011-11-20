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

#ifndef ENVIRONMENT_H__
#define ENVIRONMENT_H__

#include <vector>
#include <map>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>

#include <Model/GameObject.h>
#include <Model/Property/Value.h>

namespace BFG {

class MODEL_API Environment
{
public:
	void addGameObject(boost::shared_ptr<GameObject>);
	void removeGameObject(GameHandle);

	bool exists(GameHandle) const;

	//! Finds the GameHandle to a GameObject
	//! \param pred
	//! \return Handle of first Object which satisfies the predicate, or
	//!         NULL_HANDLE when not a single one was found.
	template <typename Pred>
	GameHandle find(Pred pred) const
	{
		GameObjectMapT::const_iterator it = mGameObjects.begin();
		for (; it != mGameObjects.end(); ++it)
		{
			boost::shared_ptr<GameObject> go = it->second.lock();
			if (go == NULL)
				continue;

			bool found = pred(go);
			if (found)
				return go->getHandle();
		}
		return NULL_HANDLE;
	}

	//! Finds the GameHandles to multiple GameObjects
	//! \param pred
	//! \return Container of all Object which satisfy the predicate.
	//!         If nothing was found, the container will be empty.
	template <typename Pred>
	std::vector<GameHandle> find_all(Pred pred) const
	{
		std::vector<GameHandle> matchingHandles;
		GameObjectMapT::const_iterator it = mGameObjects.begin();
		for (; it != mGameObjects.end(); ++it)
		{
			boost::shared_ptr<GameObject> go = it->second.lock();
			if (go == NULL)
				continue;

			bool found = pred(go);
			if (found)
				matchingHandles.push_back(go->getHandle());
		}
		return matchingHandles;
	}

	//! \see GameObject::getValue()
	template <typename RetT>
	const RetT&
	getGoValue(GameHandle handle, ValueId::VarIdT varId, PluginId pluginId) const
	{
		GameObjectMapT::const_iterator it = mGameObjects.find(handle);
		
		if (it == mGameObjects.end())
		{
			std::stringstream ss;
			ss << "Environment: Object " << handle
			   << " not found while accessing " << ValueId(varId, pluginId);
			      
			throw std::logic_error(ss.str());
		}

		boost::shared_ptr<const GameObject> go = it->second.lock();
		return go->getValue<RetT>(varId, pluginId);
	}

	GameHandle nextHandle(GameHandle current) const;
	GameHandle prevHandle(GameHandle current) const;

private:
	typedef std::map<GameHandle, boost::weak_ptr<GameObject> > GameObjectMapT;
	
	GameObjectMapT mGameObjects;
};

} // namespace BFG

#endif
