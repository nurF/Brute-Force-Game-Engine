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

#ifndef GAMEOBJECT_FACTORY_H_
#define GAMEOBJECT_FACTORY_H_

#include <map>
#include <boost/weak_ptr.hpp>

#include <EventSystem/Emitter.h>

#include <Model/Loader/Types.h>
#include <Model/Property/Plugin.h>

namespace BFG {

namespace Property
{
	class ConceptFactory;
}

class Environment;
class GameObject;
struct CameraParameter;

namespace Loader {

class Interpreter;

class MODEL_API GameObjectFactory : Emitter
{
public:
	typedef std::map<std::string, std::map<std::string, GameHandle> > GoModuleMapT;
	typedef std::map<std::string, boost::weak_ptr<GameObject> > GoMapT;

	GameObjectFactory(EventLoop* loop,
	                  const LevelConfig& files,
	                  const Property::PluginMapT& propertyPlugins,
	                  boost::shared_ptr<Interpreter> interpreter,
	                  boost::shared_ptr<Environment> environment,
	                  GameHandle stateHandle);

	boost::shared_ptr<GameObject>
	createGameObject(const ObjectParameter& parameters);

	boost::shared_ptr<GameObject>
	createCamera(const CameraParameter& cameraParameter,
                 const std::string& parentObject);

	//! Applies Connection logic of already created objects 
	void applyConnection(const ObjectParameter& parameters);

	const GoMapT& names() const { return mGameObjects; }

private:
	GoModuleMapT mGoModules;
	GoMapT mGameObjects;
	
	const Property::PluginMapT& mPropertyPlugins;

	boost::shared_ptr<Interpreter> mInterpreter;
	boost::shared_ptr<Environment> mEnvironment;
	
	typedef std::map<std::string, std::vector<ModuleParameter> > ModuleParameterMapT;
	typedef std::map<std::string, std::vector<AdapterParameter> > AdapterParameterMapT;
	typedef std::map<std::string, std::vector<ConceptParameter> > ConceptParameterMapT;
	typedef std::map<std::string, std::vector<ValueParameter> > ValueParameterMapT;
	
	ModuleParameterMapT mModuleParameters;
	AdapterParameterMapT mAdapterParameters;
	ConceptParameterMapT mConceptParameters;
	ValueParameterMapT mValueParameters;
	
	GameHandle mStateHandle;
};

} // namespace Loader
} // namespace BFG

#endif
