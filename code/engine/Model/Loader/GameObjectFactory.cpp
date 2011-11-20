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

#include <Model/Loader/GameObjectFactory.h>

#include <boost/foreach.hpp>

#include <Core/Utils.h> // generateHandle()

#include <Model/Loader/FileLoader.h>
#include <Model/Loader/Interpreter.h>
#include <Model/Adapter.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Module.h>
#include <Model/Property/Concepts/Camera.h> // struct CameraParameter

#include <Physics/Event.h>
#include <View/Event.h>

namespace BFG {
namespace Loader {

//! Utility for GameObjectFactory
template
<
	typename MapT,
	typename InterpreterT,
	typename InterpreterFunctionT
>
void interpretMmt(MapT& outMap,
                  const boost::shared_ptr<ManyManyTagsT> mmt,
                  boost::shared_ptr<InterpreterT> interpreter,
                  InterpreterFunctionT interpreterFunction)
{
	ManyManyTagsT::const_iterator mmtIt = mmt->begin(); 
	 
	for (; mmtIt != mmt->end(); ++mmtIt) 
	{ 
		boost::shared_ptr<ManyTagsT> parameterList = mmtIt->second; 

		typename MapT::mapped_type parameter;

		((*interpreter).*(interpreterFunction))(*parameterList, parameter); 
	     
		outMap.insert(std::make_pair(mmtIt->first, parameter)); 
	} 
}

GameObjectFactory::GameObjectFactory(EventLoop* loop,
                                     const LevelConfig& files,
                                     const Property::PluginMapT& propertyPlugins,
                                     boost::shared_ptr<Interpreter> interpreter,
                                     boost::shared_ptr<Environment> environment,
                                     GameHandle stateHandle) :
Emitter(loop),
mPropertyPlugins(propertyPlugins),
mInterpreter(interpreter),
mEnvironment(environment),
mStateHandle(stateHandle)
{
	FileLoader fileLoader;
	boost::shared_ptr<ManyManyTagsT> modules(new ManyManyTagsT);
	boost::shared_ptr<ManyManyTagsT> adapter(new ManyManyTagsT);
	boost::shared_ptr<ManyManyTagsT> concepts(new ManyManyTagsT);
	boost::shared_ptr<ManyManyTagsT> properties(new ManyManyTagsT);
	
	BOOST_FOREACH(const std::string& filename, files.mModules)
	{
		fileLoader.loadManyManyTagFile(filename, XmlTags::ObjectConfig, modules);
	}
	BOOST_FOREACH(const std::string& filename, files.mAdapters)
	{
		fileLoader.loadManyManyTagFile(filename, XmlTags::AdapterConfig, adapter);
	}
	BOOST_FOREACH(const std::string& filename, files.mConcepts)
	{
		fileLoader.loadManyManyTagFile(filename, XmlTags::ConceptConfig, concepts);
	}
	BOOST_FOREACH(const std::string& filename, files.mProperties)
	{
		fileLoader.loadManyManyTagFile(filename, XmlTags::ValueConfig, properties);
	}
	
	interpretMmt(mValueParameters, properties, mInterpreter, &Interpreter::interpretValueConfig);
	interpretMmt(mConceptParameters, concepts, mInterpreter, &Interpreter::interpretConceptConfig);
	interpretMmt(mAdapterParameters, adapter, mInterpreter, &Interpreter::interpretAdapterConfig);
	interpretMmt(mModuleParameters, modules, mInterpreter, &Interpreter::interpretModuleConfig);
}

boost::shared_ptr<GameObject>
GameObjectFactory::createGameObject(const ObjectParameter& parameter)
{
	GameHandle goHandle;
	if (parameter.mHandle != NULL_HANDLE)
		goHandle = parameter.mHandle;
	else
		goHandle = generateHandle();

	infolog << "GameObjectFactory: Creating GO:" << goHandle
	        << " (" << parameter.mName << ")";

	if (mModuleParameters.find(parameter.mType) == mModuleParameters.end())
		throw std::runtime_error("GameObjectFactory::createGameObject(): "
			"Type \"" + parameter.mType + "\" not found!");

	// Start Position and Orientation is passed with the GO-Waypoint
	Location location(parameter.mLocation);

	// Create the PhysicsObject
	Physics::ObjectCreationParams ocp(goHandle, parameter.mLocation);
	emit<Physics::Event>(ID::PE_CREATE_OBJECT, ocp);

	// First Module is always root
	bool isRoot = true;
	
	boost::shared_ptr<GameObject> gameObject;

	// In order to connect Modules together, we need the GameHandles of
	// previously created modules.
	std::map<std::string, GameHandle> moduleNameHandleMap;
	
	// Create all Modules
	if (mModuleParameters[parameter.mType].empty())
		throw std::runtime_error("GameObjectFactory::createGameObject(): "
			"GameObjects must have at least one Module!");
	
	std::vector<ModuleParameter>::iterator moduleIt = mModuleParameters[parameter.mType].begin();
	for (; moduleIt != mModuleParameters[parameter.mType].end(); ++moduleIt)
	{
		ModuleParameter& moduleParameter = *moduleIt;

		GameHandle moduleHandle;

		// The root module and its owner GameObject
		// must share the same GameHandle.
		if (isRoot)
			moduleHandle = goHandle;
		else
			moduleHandle = generateHandle();

		boost::shared_ptr<Module> module(new Module(moduleHandle));

		bool isVirtual = moduleParameter.mMeshName.empty();

		if (! isVirtual)
		{
			// Physical representation			
			Physics::ModuleCreationParams mcp
			(
				goHandle,
				moduleHandle,
				moduleParameter.mMeshName,
				moduleParameter.mCollisionMode,
				v3::ZERO,
				qv4::IDENTITY,
				moduleParameter.mDensity
			);

			emit<Physics::Event>(ID::PE_ATTACH_MODULE, mcp);

			// Visual representation
			View::ObjectCreation oc
			(
				NULL_HANDLE,
				moduleHandle,
				moduleParameter.mMeshName,
				v3::ZERO,
				qv4::IDENTITY
			);
	
			emit<View::Event>(ID::VE_CREATE_OBJECT, oc, mStateHandle);

			if (!isRoot)
			{
				emit<View::Event>(ID::VE_ATTACH_OBJECT, moduleHandle, goHandle);
			}
		}

		// Load "Concepts" and their "Values"
		if (mConceptParameters[moduleParameter.mConcepts].empty())
		{
			throw std::runtime_error("GameObjectFactory::createGameObject(): "
				"Concepts of \"" + moduleParameter.mConcepts + "\" not found!");
		}

		std::vector<ConceptParameter>::iterator conceptIt = 
			mConceptParameters[moduleParameter.mConcepts].begin();

		for (; conceptIt != mConceptParameters[moduleParameter.mConcepts].end(); ++conceptIt)
		{
			ConceptParameter& conceptParameter = *conceptIt;
			std::vector<ValueParameter>::iterator valueIt = 
				mValueParameters[conceptParameter.mProperties].begin();
				
			for (; valueIt != mValueParameters[conceptParameter.mProperties].end(); ++valueIt)
			{
				ValueParameter& valueParameter = *valueIt;
				module->mValues[valueParameter.mId] = valueParameter.mValue;
			}
			
			module->mPropertyConcepts.push_back(conceptParameter.mId);
		}
		
		// Store GameHandle for later use
		moduleNameHandleMap[moduleParameter.mName] = moduleHandle;
		
		if (isRoot)
		{
			if (! isVirtual)
			{
				emit<Physics::Event>(ID::PE_UPDATE_VELOCITY, parameter.mLinearVelocity, goHandle);
				emit<Physics::Event>(ID::PE_UPDATE_ROTATION_VELOCITY, parameter.mAngularVelocity, goHandle);
			}
			
			// Create GameObject
			gameObject.reset
			(
				new GameObject
				(
					loop(),
					goHandle,
					parameter.mName,
					mPropertyPlugins,
					mEnvironment
				)
			);

			// Register it in the environment
			mEnvironment->addGameObject(gameObject);
		}

		GameHandle parentHandle;
		
		if (moduleParameter.mConnection.mConnectedExternToModule.empty())
			parentHandle = NULL_HANDLE;
		else
			parentHandle = moduleNameHandleMap[moduleParameter.mConnection.mConnectedExternToModule];

		if (isVirtual)
		{
			gameObject->attachModule(module);
		}
		else
		{
			std::vector<Adapter> adapterVector;

			std::vector<AdapterParameter>::iterator adapterIt = 
				mAdapterParameters[moduleParameter.mAdapters].begin();

			for (; adapterIt != mAdapterParameters[moduleParameter.mAdapters].end(); ++adapterIt)
			{
				AdapterParameter& adapterParameter = *adapterIt;
				
				Adapter adapter;
				adapter.mParentPosition = adapterParameter.mLocation.position;
				adapter.mParentOrientation = adapterParameter.mLocation.orientation;
				adapter.mIdentifier = adapterParameter.mId;
				
				adapterVector.push_back(adapter);
			}
		
			gameObject->attachModule
			(
				module,
				adapterVector,
				moduleParameter.mConnection.mConnectedLocalAt,
				parentHandle,
				moduleParameter.mConnection.mConnectedExternAt
			);
		}
		
		isRoot = false;	
	}

	mGoModules[parameter.mName] = moduleNameHandleMap;
	mGameObjects[parameter.mName] = gameObject;
	
	return gameObject;
}

boost::shared_ptr<GameObject>
GameObjectFactory::createCamera(const CameraParameter& cameraParameter,
                                const std::string& parentObject)
{
	GoMapT::const_iterator it =
		mGameObjects.find(parentObject);
		
	if (it == mGameObjects.end())
	{
		throw std::runtime_error
		(
			"GameObjectFactory: Unable to find \"" + parentObject + "\" for "
			"use as camera position. Skipping the creation of this camera."
		);
	}

	GameHandle parentHandle = it->second.lock()->getHandle();

	GameHandle camHandle = generateHandle();

	Physics::ObjectCreationParams ocp(camHandle, Location());
	emit<Physics::Event>(ID::PE_CREATE_OBJECT, ocp);

	Physics::ModuleCreationParams mcp
	(
		camHandle,
		camHandle,
		"cube.mesh",
		ID::CM_Disabled,
		v3::ZERO,
		qv4::IDENTITY,
		50.0f
	);

	emit<Physics::Event>(ID::PE_ATTACH_MODULE, mcp);
	
	boost::shared_ptr<GameObject> camera
	(
		new GameObject
		(
			loop(),
			camHandle,
			"Camera",
			mPropertyPlugins,
			mEnvironment
		)
	);

	boost::shared_ptr<Module> camModule(new Module(camHandle));
	camModule->mPropertyConcepts.push_back("Physical");
	camModule->mPropertyConcepts.push_back("Camera");
	
	camModule->mValues[ValueId(ID::PV_CameraMode, ValueId::ENGINE_PLUGIN_ID)] = cameraParameter.mMode;
	camModule->mValues[ValueId(ID::PV_CameraOffset, ValueId::ENGINE_PLUGIN_ID)] = cameraParameter.mOffset;

	camera->attachModule(camModule);

	View::CameraCreation cc(camHandle, NULL_HANDLE, cameraParameter.mFullscreen, 0, 0);
	emit<View::Event>(ID::VE_CREATE_CAMERA, cc, mStateHandle);
	emit<GameObjectEvent>(ID::GOE_SET_CAMERA_TARGET, parentHandle, camHandle);
	
	mEnvironment->addGameObject(camera);
	
	return camera;
}

void GameObjectFactory::applyConnection(const ObjectParameter& parameters)
{
	if (! parameters.mConnection.good())
		return;

	GoMapT::const_iterator it =
		mGameObjects.find(parameters.mConnection.mConnectedExternToGameObject);

	// Check if parent gameobject exists at all
	if (it == mGameObjects.end())
	{
		errlog << "GameObjectFactory: Unable to connect \"" << parameters.mName
		       << "\" to " << "\""
		       << parameters.mConnection.mConnectedExternToGameObject
		       << "\" since the latter wasn't found.";
		return;
	}

	boost::shared_ptr<GameObject> parent = it->second.lock();
	
	GameHandle parentModule = 
		mGoModules[parameters.mConnection.mConnectedExternToGameObject]
		          [parameters.mConnection.mConnectedExternToModule];

	u32 parentAdapter = parameters.mConnection.mConnectedExternAt;

	boost::shared_ptr<GameObject> child = mGameObjects[parameters.mName].lock();
	
	u32 childAdapter = parameters.mConnection.mConnectedLocalAt;
	
	parent->attachModule
	(
		child,
		child->rootAdapters(),
		childAdapter,
		parentModule,
		parentAdapter
	);
}

} // namespace Loader
} // namespace BFG
