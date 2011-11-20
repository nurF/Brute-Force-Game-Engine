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

#include <Model/Sector.h>

#include <EventSystem/Core/EventLoop.h>

#include <Model/Events/SectorEvent.h>
#include <Model/GameObject.h>
#include <Model/Loader/GameObjectFactory.h>

#include <Physics/Event.h>
#include <View/Event.h>

namespace BFG {

Sector::Sector(EventLoop* loop,
               GameHandle handle,
               const std::string& name,
               boost::shared_ptr<Loader::GameObjectFactory> gof) :
Managed(handle, name, ID::OT_Sector),
Emitter(loop),
mGameObjectFactory(gof)
{
	assert(gof && "GameObjectFactory must be initialized");

	loop->connect(ID::S_CREATE_GO, this, &Sector::onCreateObject);
	loop->connect(ID::S_DESTROY_GO, this, &Sector::onDestroyObject);
}

Sector::~Sector()
{
	loop()->disconnect(ID::S_CREATE_GO, this);
	loop()->disconnect(ID::S_DESTROY_GO, this);
}

void Sector::addObject(boost::shared_ptr<Managed> object)
{
	// Check if object already exists
	if(mObjectMap.find(object->getHandle()) != mObjectMap.end())
	{
		std::stringstream ss;
		ss << "Sector::addObject: Object with the same ID: \""
		   << object->getHandle()
		   << "\" already exists:";
		throw std::logic_error(ss.str());
	}

	mObjectMap[object->getHandle()] = object; 
}

void Sector::removeObject(GameHandle handle)
{
	assert(mObjectMap.find(handle) != mObjectMap.end() &&
	       "Tried to delete something which doesn't exist.");

	// Delayed removal
	mToRemove.push_back(handle);
}

void Sector::internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
{
	deleteMarkedObjectsForRemoval();

	ObjectMapT::iterator it = mObjectMap.begin();
	for(; it != mObjectMap.end(); ++it)
	{
		it->second->update(timeSinceLastFrame);
	}
}

void Sector::deleteMarkedObjectsForRemoval()
{
	for(size_t i=0; i<mToRemove.size(); ++i)
	{
		mObjectMap.erase(mToRemove[i]);
		
		emit<View::Event>(ID::VE_DESTROY_OBJECT, mToRemove[i]);
		emit<Physics::Event>(ID::PE_DELETE_OBJECT, mToRemove[i]);
	}
	mToRemove.clear();
}

void Sector::onCreateObject(SectorEvent* se)
{
	boost::shared_ptr<GameObject> go = mGameObjectFactory->createGameObject
	(	
		boost::get<Loader::ObjectParameter>(se->getData())
	);
	
	addObject(go);
}

void Sector::onDestroyObject(SectorEvent* se)
{
	GameHandle handle = boost::get<GameHandle>(se->getData());
	removeObject(handle);
}

} // namespace BFG
