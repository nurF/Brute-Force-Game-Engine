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

#include <Model/Environment.h>
#include <Model/GameObject.h>

namespace BFG {

void Environment::addGameObject(boost::shared_ptr<GameObject> go)
{
	if (go->getHandle() == NULL_HANDLE)
		throw std::logic_error
			("Environment: Tried to add a GameObject with an NULL_HANDLE.");

	mGameObjects[go->getHandle()] = go;
}

void Environment::removeGameObject(GameHandle handle)
{
	mGameObjects.erase(handle);
}

bool Environment::exists(GameHandle handle) const
{
	return mGameObjects.find(handle) != mGameObjects.end();
}

GameHandle Environment::nextHandle(GameHandle current) const
{
	GameObjectMapT::const_iterator it;
	// when there was no handle get the first one
	if (current == NULL_HANDLE)
	{
		it = mGameObjects.begin();
		if (it != mGameObjects.end())
			return it->first;
		return NULL_HANDLE;
	}

	it = mGameObjects.find(current);
	if (it != mGameObjects.end())
	{
		// when the current is the last one
		if (it == (--mGameObjects.end()))
		{
			// get the first
			it = mGameObjects.begin();
			return it->first;
		}
		++it;
		return it->first;
	}

	return NULL_HANDLE;
}

GameHandle Environment::prevHandle(GameHandle current) const
{
	GameObjectMapT::const_iterator it;
	// when there was no handle get the last one
	if (current == NULL_HANDLE)
	{
		if (mGameObjects.size() > 0)
		{
			it = --mGameObjects.end();
			return it->first;
		}
		return NULL_HANDLE;
	}

	it = mGameObjects.find(current);
	if (it != mGameObjects.end())
	{
		// when the current is the first one
		if (it == mGameObjects.begin())
		{
			// get the last
			it = --mGameObjects.end();
			return it->first;
		}
		--it;
		return it->first;
	}

	return NULL_HANDLE;
}

} // namespace BFG
