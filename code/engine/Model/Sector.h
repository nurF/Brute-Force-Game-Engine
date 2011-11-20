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

#ifndef BFG_SECTOR_H__
#define BFG_SECTOR_H__

#include <string>
#include <map>

#include <boost/shared_ptr.hpp>

#include <boost/units/systems/si/time.hpp>
#include <boost/units/quantity.hpp>

#include <EventSystem/Emitter.h>
#include <Core/Types.h>

#include <Model/Events/SectorEvent_fwd.h>
#include <Model/Managed.h>

using namespace boost::units;


namespace BFG {

namespace Loader {
	class GameObjectFactory;
}

class GameObject;

//! Dynamic creation and destruction observer
class MODEL_API Sector : public Managed,
                         Emitter
{
public:
	typedef std::map<GameHandle, boost::shared_ptr<Managed> > ObjectMapT;

	Sector(EventLoop* loop,
	       GameHandle handle,
	       const std::string& name,
	       boost::shared_ptr<Loader::GameObjectFactory> gof);
	       
	~Sector();

	//! \brief Add an object to sector (takes ownership)
	//! \param object Pointer to an object which must exist
	void addObject(boost::shared_ptr<Managed> object);

	//! \brief Remove an object from sector
	//! \param handle Handle of an object which must exist in the sector
	void removeObject(GameHandle handle);

private:
	void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);

	void deleteMarkedObjectsForRemoval();

	void onCreateObject(SectorEvent* goe);
	void onDestroyObject(SectorEvent* goe);

	ObjectMapT                                   mObjectMap;
	std::vector<GameHandle>                      mToRemove;
	boost::shared_ptr<Loader::GameObjectFactory> mGameObjectFactory;
};

} // namespace BFG

#endif
