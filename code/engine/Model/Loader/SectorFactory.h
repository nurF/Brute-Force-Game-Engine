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

#ifndef SECTOR_FACTORY_H_
#define SECTOR_FACTORY_H_

#include <map>
#include <EventSystem/Emitter.h>
#include <Model/Loader/Types.h>

namespace BFG {

class Sector;
class GameObject;
class Environment;
struct Waypoint;
struct CameraParameter;

namespace Loader {

class Interpreter;
class GameObjectFactory;

class MODEL_API SectorFactory : Emitter
{
	typedef boost::unordered_map<std::string, GameHandle> NameToHandleT;
	typedef std::pair<std::string, GameHandle> LinkT;
	typedef boost::unordered_map<GameHandle, boost::shared_ptr<Waypoint> > WaypointMapT; // <owner, waypoint>

public:
	//! \param[in] interpreter Must point to a constructed interpreter
	//! \param[in] gof         Must point to a constructed GameObject factory
	//! \param[in] env         Must point to a constructed Environment
	//! \param[in] viewState   Design Hack: View entities are created by event.
	//!                        This handle is the destination for those events.
	SectorFactory(EventLoop* loop,
	              boost::shared_ptr<Interpreter> interpreter,
	              boost::shared_ptr<GameObjectFactory> gof,
	              boost::shared_ptr<Environment> env,
	              GameHandle viewState);

	boost::shared_ptr<Sector>
	createSector(const std::string& fileSectorConfig,
	             GameHandle& outCameraHandle);

#if 0
	GameHandle createWaypoint(const std::string& objectName);
	GameHandle createWaypoint(const v3& position);

	void createPath(const ManyTagsT& pathDefinition, std::vector<GameHandle>& path);
#endif

#if 0
	void createLaserP(const Location& loc, const v3& startVelocity);
	void createRocket(const Location& loc,
	                  const v3& startVelocity,
	                  GameHandle target);
#endif

private:
	typedef std::map<std::string, std::vector<ObjectParameter> > ObjectParameterMapT;

	boost::shared_ptr<Interpreter>       mInterpreter;
	boost::shared_ptr<GameObjectFactory> mGameObjectFactory;
	boost::shared_ptr<Environment>       mEnvironment;
	GameHandle                           mViewState;

	NameToHandleT mObjectHandleStringMap;
	WaypointMapT  mWaypointMap;

	ManyManyTagsT mObjectTypeStrings;
};

} // namespace Loader
} // namespace BFG

#endif
