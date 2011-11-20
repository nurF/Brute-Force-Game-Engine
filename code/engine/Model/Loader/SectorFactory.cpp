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

#include <Model/Loader/SectorFactory.h>

#include <boost/foreach.hpp>

#include <Core/Utils.h> // generateHandle()

#include <Model/Loader/FileLoader.h>
#include <Model/Loader/GameObjectFactory.h>
#include <Model/Loader/Interpreter.h>
#include <Model/GameObject.h>
#include <Model/Sector.h>
#include <Model/Property/Concepts/Camera.h> // struct CameraParameter

#include <View/Event.h>

namespace BFG {
namespace Loader {

SectorFactory::SectorFactory(EventLoop* loop,
                             boost::shared_ptr<Interpreter> interpreter,
                             boost::shared_ptr<GameObjectFactory> gof,
                             boost::shared_ptr<Environment> environment,
                             GameHandle viewState) :
Emitter(loop),
mInterpreter(interpreter),
mGameObjectFactory(gof),
mEnvironment(environment),
mViewState(viewState)
{
	assert(gof && "Interpreter must be initialized");
	assert(gof && "GameObjectFactory must be initialized");
	assert(environment && "Environment must be initialized");
}

boost::shared_ptr<Sector>
SectorFactory::createSector(const std::string& fileSectorConfig,
							GameHandle& outCameraHandle)
{
	FileLoader fileLoader;

	SectorDraft sectorDraft;
	sectorDraft.mObjects.reset(new ManyTagsT);
	sectorDraft.mPaths.reset(new ManyManyTagsT);
	sectorDraft.mSectorProperties.reset(new TagWithAttributesT);
	sectorDraft.mConditions.reset(new TagWithAttributesT);
	sectorDraft.mCameras.reset(new ManyTagsT);
	sectorDraft.mSkybox.reset(new TagWithAttributesT);
	sectorDraft.mLights.reset(new ManyTagsT);
	sectorDraft.mPlaylist.reset(new ManyTagsT);
	
	// Build Sector
	std::string sectorName;

	fileLoader.loadSectorDraft(fileSectorConfig, sectorDraft, sectorName);

	boost::shared_ptr<Sector> sector(new Sector
	(
		loop(),
		generateHandle(),
		sectorName,
		mGameObjectFactory
	));

	// Build Objects
	std::vector<ObjectParameter> objVector;
	mInterpreter->interpretObjectConfig(*sectorDraft.mObjects, objVector);
	
	BOOST_FOREACH(ObjectParameter& op, objVector)
	{
		sector->addObject(mGameObjectFactory->createGameObject(op));
	}

	BOOST_FOREACH(ObjectParameter& op, objVector)
	{
		mGameObjectFactory->applyConnection(op);
	}

	// Setup View
	emit<View::Event>(ID::VE_SET_AMBIENT, cv4(0.1f, 0.1f, 0.1f));

	TagWithAttributesT::iterator skyIt = sectorDraft.mSkybox->begin();
	if (skyIt == sectorDraft.mSkybox->end())
	{
		throw std::runtime_error("CreateSector: no skybox material defined!");
	}
	
	View::SkyCreation sc(skyIt->second);
	emit<View::Event>(ID::VE_SET_SKY, sc, mViewState);
	
	// Create all lights
	BOOST_FOREACH(ManyTagsT::value_type em, *sectorDraft.mLights)
	{
		LightParameters lightParameters;

		mInterpreter->interpretLightDefinition(*em, lightParameters);
	
		View::DirectionalLightCreation dlc(generateHandle(), lightParameters.mOrientation);
		emit<View::Event>(ID::VE_CREATE_DIRECTIONALLIGHT, dlc);
	}

	// Create all cameras
	BOOST_FOREACH(ManyTagsT::value_type em, *sectorDraft.mCameras)
	{
		CameraParameter cameraParameter;
		std::string parentObject;

		mInterpreter->interpretCameraDefinition(*em, cameraParameter, parentObject);

		boost::shared_ptr<GameObject> cam =
			mGameObjectFactory->createCamera(cameraParameter, parentObject);
			
		sector->addObject(cam);
	}

#if 0
	// Create all paths
	typedef boost::unordered_map<std::string, std::vector<GameHandle> > PathListT;
	PathListT pathList;

	BOOST_FOREACH(ManyManyTagsT::value_type paths, *sectorDraft.mPaths)
	{
		std::vector<GameHandle> aPath;
		createPath(*paths.second, aPath);
		pathList.insert(std::make_pair(paths.first, aPath));
	}

	// Create RaceControl
	RaceCondition raceCondition;
	std::vector<GameHandle> path;
	std::string pathId;

	mInterpreter->interpretRaceCondition(*sectorDraft.mConditions, raceCondition, pathId);

	if (! pathId.empty())
	{
		PathListT::iterator pathIt = pathList.find(pathId);

		// Check: If it has a name, it should be there.
		if (pathIt == pathList.end())
			throw LoaderException("SectorFactory: Path \"" + pathId + "\"not found in path list.");
			
		// This is a copy, but that's okay. If the pathId is empty, we'll pass
		// an empty vector to the race control ctor to ensure proper
		// initialization.
		path = pathIt->second;

		outRaceControl.reset(new RaceControl
		(
			generateHandle(),
			path,
			raceCondition, 
			*mEnvironment
		));
	}

	// Create the playlist for music
	outPlaylistControl.reset(new PlaylistControl
	(
		GameStateManager::getSingleton().getEventLoop()
	));

	BOOST_FOREACH(ManyTagsT::value_type song, *sectorDraft.mPlaylist)
	{
		std::string publicName, fileName;

		mInterpreter->interpretPlaylistDefinition(*song, publicName, fileName);

		outPlaylistControl->addSong(publicName, fileName);
	}	
#endif

	return sector;

#if 0
	// create raceControl
	RaceCondition raceCondition;
	std::string path;

	mInterpreter->interpretRaceCondition(draft.mConditions, raceCondition, path);
	
	PathListT::iterator pathIt = pathList.find(path);
	if (pathIt == pathList.end())
		throw LoaderException("Path not found in path list.");

	std::vector<GameHandle> waypoints;


	// waypoint list for racetrack.
	BOOST_FOREACH(GameHandle gh, pathIt->second)
	{
		WaypointMapT::iterator waypointIt = mWaypointMap.find(gh);
		waypoints.push_back(waypointIt->second->getHandle());
	}

	mRaceControl.reset(new RaceControl(generateHandle(),
	                                   pathIt->second, 
	                                   raceCondition, 
	                                   *mEnvironment));



	View::Gui* gui = new View::Gui(GameStateManager::getSingleton().getEventLoop(),
                                   player);


	outPlaylistControl.reset(new PlaylistControl(GameStateManager::getSingleton().getEventLoop()));

	// create the playlist for music.
	BOOST_FOREACH(ListOfEntitiesT::value_type song, draft.mPlaylist)
	{
		std::string publicName, fileName;
	
		mInterpreter->interpretPlaylistDefinition(*song, publicName, fileName);

		outPlaylistControl->addSong(publicName, fileName);
	}	

	//! \TODO This doesn't belong to this place. Put it into view.

	// ######################
	// # Pre-load Explosion #
	// ######################

	mRenderInt->loadMaterial("explosion", "General");
	mRenderInt->loadMaterial("explosion2", "General");
	mRenderInt->loadMaterial("flash", "General");
	mRenderInt->loadMaterial("spark", "General");
	mRenderInt->loadMaterial("smoketrail", "General");
	mRenderInt->loadMaterial("shockwave", "General");

	// ##################
	// # Pre-load Laser #
	// ##################

	mRenderInt->loadMaterial("laser_front", "General");
	mRenderInt->loadMaterial("laser_side", "General");
#endif
}

#if 0
// This function affect to create a Waypoint-Object, but it only returns the waypoint of
// an existing object. This is needed to create paths by objects.
GameHandle SectorFactory::createWaypoint(const std::string& objectName)
{
	GameObjectFactory::GoMapT::const_iterator it;
	
	it = mGameObjectFactory->names().find(objectName);

	if (it == mGameObjectFactory->names().end())
	{
		std::stringstream ss;
		ss << "createWaypoint: " << objectName << " not found.";
		throw LoaderException(ss.str());
	}
	
	boost::shared_ptr<GameObject> go = it->second.lock();
	
	if (! go)
	{
		std::stringstream ss;
		ss << "createWaypoint: " << objectName << " does not exist anymore.";
		throw LoaderException(ss.str());
	}
	
	return go->getHandle();
}

// This function creates a lonely waypoint just by a position vector. 
// This waypoint is not related to any object. 
GameHandle SectorFactory::createWaypoint(const v3& position)
{
	ObjectParameter parameters;
	parameters.mLocation.position = position;

	return createGameObject(parameters);
}

void SectorFactory::createPath(const ManyTagsT& pathDefinition, std::vector<GameHandle>& path)
{
	BOOST_FOREACH(ManyTagsT::value_type em, pathDefinition)
	{
		path.push_back(mInterpreter->interpretPathDefinition(em->begin()->second, *this));
	}
}
#endif


#if 0
void SectorFactory::createLaserP(const Location& loc,
								 const v3& startVelocity)
{
	View::RenderInterface* renderInt = View::RenderInterface::Instance();
	View::ObjectInterface* viewObjInt = View::ObjectInterface::Instance();
	Physics::PhysicsManager* physicsMngr = Physics::PhysicsManager::getSingletonPtr();

	GameHandle handle = generateHandle();

	boost::shared_ptr<Waypoint> wpLaser(new Waypoint(generateHandle(), loc, handle));

	// This has to be done in advance of the GameObject creation due to call of setPosition and setOrientation
	physicsMngr->createPhysicalObject(
		handle,
		ID::CG_Sphere,
		0.5f,			//radius of sphere
		ID::CT_Bullet,	// This has nothing to say. It's not even used.
		true,
		true
	);

	boost::shared_ptr<ViewUpdateBase> view_updater(new ViewObjectUpdater);

	viewObjInt->createObject(handle, NULL_HANDLE, true);

	boost::shared_ptr<GameObject> laser(
		new GameObject(
		handle,
		"Laser",
		mEnvironment,
		wpLaser,
		view_updater
		)
	);
	
	mEnvironment->addGameObject(laser);

	boost::shared_ptr<Physical> h;
	h.reset(new Physical(*laser, 100.0f, 23.75f));
	boost::shared_static_cast<PropertyConcept>(h)->activate();
	laser->addPropertyConcept(h);

	NoRespawn respawn(*mSector);

	boost::shared_ptr<Destroyable<NoRespawn> > d;
	d.reset(new Destroyable<NoRespawn>(*laser, 5, respawn));
	boost::shared_static_cast<PropertyConcept>(d)->activate();
	laser->addPropertyConcept(d);

	boost::shared_ptr<SelfDestruction> s;
	s.reset(new SelfDestruction(*laser, 2.0f));
	boost::shared_static_cast<PropertyConcept>(s)->activate();
	laser->addPropertyConcept(s);

	boost::shared_ptr<Engine> e;
	e.reset(new Engine(*laser, startVelocity.squaredLength(), v3(0, 0, 0), 0.0f, 0.0f, 0.01f, 0.01f, 200.f, 10.f, startVelocity)); 
	boost::shared_static_cast<PropertyConcept>(e)->activate();
	laser->addPropertyConcept(e);

#ifdef DISCO // DISCO! YEAH BABY!
	static int disco = 0;

	if (disco == 0) viewObjInt->createLaserP(handle, loc, startVelocity, cv4::Black);
	else if (disco == 1) viewObjInt->createLaserP(handle, loc, startVelocity, cv4::Blue);
	else if (disco == 2) viewObjInt->createLaserP(handle, loc, startVelocity, cv4::Green);
	else if (disco == 3) viewObjInt->createLaserP(handle, loc, startVelocity, cv4::Red);
	else if (disco == 4) viewObjInt->createLaserP(handle, loc, startVelocity, cv4::White);
	else if (disco == 5) viewObjInt->createLaserP(handle, loc, startVelocity, Ogre::ColourValue(0.0f, 1.0f, 1.0f));
	else if (disco == 6) viewObjInt->createLaserP(handle, loc, startVelocity, Ogre::ColourValue(1.0f, 1.0f, 0.0f));

	if (++disco == 7)
		disco = 0;
#else
	viewObjInt->createLaserP(handle, loc, startVelocity, cv4::Green);
#endif

	mSector->addObject(laser);
}

void SectorFactory::createRocket(const Location& loc,
								 const v3& startVelocity,
								 GameHandle target)
{
	View::RenderInterface* renderInt = View::RenderInterface::Instance();
	View::ObjectInterface* viewObjInt = View::ObjectInterface::Instance();
	Physics::PhysicsManager* physicsMngr = Physics::PhysicsManager::getSingletonPtr();

	GameHandle handle = generateHandle();

	boost::shared_ptr<Waypoint> wpRocket(new Waypoint(generateHandle(), loc, handle));

	// This has to be done in advance of the GameObject creation due to call of setPosition and setOrientation
	physicsMngr->createPhysicalObject(
		handle,
		ID::CG_TriMesh,
		"Rocket.mesh",
		ID::CT_Bullet,	// This has nothing to say. It's not even used.
		true,
		true
	);

	boost::shared_ptr<ViewUpdateBase> view_updater(new ViewObjectUpdater);

	viewObjInt->createObject(
		handle,
		NULL_HANDLE,
		true,
		loc.position,
		loc.orientation
	);

	boost::shared_ptr<GameObject> rocket1(
		new GameObject(
		handle,
		"Rocket",
		mEnvironment,
		wpRocket,
		view_updater
		)
	);
	
	mEnvironment->addGameObject(rocket1);

	const float max_speed = 1e5;
	const v3 maxAngularVelocity = v3(M_PI, M_PI, M_PI);
	
	boost::shared_ptr<EnergyCell> Ec;	
	Ec.reset(new EnergyCell(*rocket1, 200.f, 2000.f));
	boost::shared_static_cast<PropertyConcept>(Ec)->activate();
	rocket1->addPropertyConcept(Ec);

	boost::shared_ptr<EnergyAllocator> Ea;	
	Ea.reset(new EnergyAllocator(*rocket1));
	boost::shared_static_cast<PropertyConcept>(Ea)->activate();
	rocket1->addPropertyConcept(Ea);

	boost::shared_ptr<Physical> h;	
	h.reset(new Physical(*rocket1, 100.0f, 23.75f));
	boost::shared_static_cast<PropertyConcept>(h)->activate();
	rocket1->addPropertyConcept(h);

	boost::shared_ptr<Engine> e;
	e.reset(new Engine(*rocket1, max_speed, maxAngularVelocity, 10417.0f, 300.0f, 1.00f, 0.01f, 200.f, 10.f, startVelocity));
	boost::shared_static_cast<PropertyConcept>(e)->activate();
	rocket1->addPropertyConcept(e);

	boost::shared_ptr<AutoNavigator> n;
	n.reset(new AutoNavigator(*rocket1, 0.01745f, max_speed, maxAngularVelocity, 1.0f));
	boost::shared_static_cast<PropertyConcept>(n)->activate();
	rocket1->addPropertyConcept(n);

	NoRespawn respawn(*mSector);

	boost::shared_ptr<Destroyable<NoRespawn> > d;
	d.reset(new Destroyable<NoRespawn>(*rocket1, 5, respawn));
	boost::shared_static_cast<PropertyConcept>(d)->activate();
	rocket1->addPropertyConcept(d);

	boost::shared_ptr<SelfDestruction> s;
	s.reset(new SelfDestruction(*rocket1, 20.0f));
	boost::shared_static_cast<PropertyConcept>(s)->activate();
	rocket1->addPropertyConcept(s);

	viewObjInt->createOgreEntity(handle, "Rocket.mesh");
	viewObjInt->attachEntityToObject(handle, handle);

#ifdef DISCO // DISCO! YEAH BABY!
	static int disco = 0;

	if (disco == 0) renderInt->attachSmokeTrail(handle, v3(0,0,-1), Ogre::ColourValue(1.0f, 0.0f, 1.0f));
	else if (disco == 1) renderInt->attachSmokeTrail(handle, v3(0,0,-1), cv4::Blue);
	else if (disco == 2) renderInt->attachSmokeTrail(handle, v3(0,0,-1), cv4::Green);
	else if (disco == 3) renderInt->attachSmokeTrail(handle, v3(0,0,-1), cv4::Red);
	else if (disco == 4) renderInt->attachSmokeTrail(handle, v3(0,0,-1), cv4::White);
	else if (disco == 5) renderInt->attachSmokeTrail(handle, v3(0,0,-1), Ogre::ColourValue(0.0f, 1.0f, 1.0f));
	else if (disco == 6) renderInt->attachSmokeTrail(handle, v3(0,0,-1), Ogre::ColourValue(1.0f, 1.0f, 0.0f));

	if (++disco == 7)
		disco = 0;
#else
	renderInt->attachSmokeTrail(handle, v3(0,0,-1), cv4::White);
#endif

	// Body Module
	boost::shared_ptr<Module> Body = new Module;

	Body->mHandle = generateHandle();
	Body->mValues[ID::PV_Weight] = 1000.0f;
	Body->mValues[ID::PV_MaxSpeed] = 750.0f;
	
	// Body Adapters
	Adapter Active;
	Adapter Inactive;

	Active.mParentPosition = v3(1.0f, 0, 0);
	Active.mParentOrientation = qv4(1,2,3,4);
	Active.mIdentifier = 3;

	Inactive.mParentPosition = v3(0, -1.0f, 0);
	Inactive.mParentOrientation = qv4(4,3,2,1);
	Inactive.mIdentifier = 4;

	
	std::vector<Adapter> Adapters;
	Adapters.push_back(Active);
	Adapters.push_back(Inactive);
	
	// Attach Body
	rocket1->attachModule(Body, Adapters, 0, NULL_HANDLE, 0);

	// Wing Module
	boost::shared_ptr<Module> Wing = new Module;
	
	Wing->mHandle = generateHandle();
	Wing->mValues[ID::PV_Weight] = 200.0f;

	// Wing Adapters
	Adapter Active2;
	Active2.mParentPosition = v3(-1.0f, 0, 0);
	Active2.mParentOrientation = qv4(5,6,7,8);
	Active2.mIdentifier = 1;
	
	Adapters.clear();
	Adapters.push_back(Active2);
	
	// Attach Wing to Body
	rocket1->attachModule(Wing, Adapters, 1, Body->mHandle, 3);

	rocket1->detachModule(Wing->mHandle);

	mSector->addObject(rocket1);
// 
// //! \todo put this into weapon rack.
// 	GameObjectEventEmitter::emit
// 	(
// 		ID::GOE_AUTONAVIGATE,
// 		target,
// 		handle
// 	);

}
#endif

} // namespace Loader
} // namespace BFG

