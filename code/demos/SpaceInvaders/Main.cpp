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

#include <OgreException.h>

#include <boost/units/systems/si/velocity.hpp>
#include <boost/units/systems/si/length.hpp>

#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreParticleSystem.h>

#include <EventSystem/EventFactory.h>
#include <Controller/Action.h>
#include <Controller/ControllerEvents.h>
#include <Controller/ControllerInterface.h>
#include <Base/CEntryPoint.h>
#include <Base/CLogger.h>
#include <Base/Cpp.h>
#include <Base/Pause.h>
#include <EventSystem/Core/EventLoop.h>
#include <EventSystem/Emitter.h>
#include <Core/ShowException.h>
#include <Core/Types.h>
#include <Core/Math.h>
#include <Core/Path.h>
#include <Core/ClockUtils.h>
#include <Core/Utils.h>
#include <View/Owner.h>
#include <View/Interface.h>
#include <View/Event.h>
#include <View/State.h>
#include <View/RenderObject.h>
#include <View/Main.h>
#include <View/Explosion.h>
#include <View/SkyCreation.h>
#include <View/Skybox.h>
#include <Model/Interface.h>
#include <Model/Loader/Interpreter.h>
#include <Model/Loader/GameObjectFactory.h>
#include <Model/Environment.h>
#include <Model/GameObject.h>
#include <Model/Module.h>
#include <Model/Sector.h>
#include <Model/Property/Concept.h>
#include <Model/Property/SpacePlugin.h>
#include <Physics/Event.h>
#include <Physics/Interface.h>
#include <AudioBlaster/AudioBlaster.h>
#include <AudioBlaster/Transport.h>
#include <AudioBlaster/Interface.h>
#include <AudioBlaster/Enums.hh>


using namespace BFG;

const f32 OBJECT_Z_POSITION = 80.0f;
const f32 DISTANCE_TO_WALL = 40.0f;
const f32 NEGATIVE_SHIP_Y_POSITION = 35.0f;

const f32 INVADER_MARGIN_X = 7.0f;
const f32 INVADER_MARGIN_Y = 8.0f;
const f32 INVADER_BOTTOM_MARGIN = 25.0f;
const f32 INVADERS_PER_ROW = 11.0f;
const f32 INVADERS_PER_COL = 5.0f;

const f32 SHIP_SPEED_MULTIPLIER = 25.0f;

const ValueId::VarIdT ID_PROJECTILE_SPEED          = 1;
const ValueId::VarIdT ID_PROJECTILE_SPAWN_DISTANCE = 2;

const v3 INVADER_VELOCITY = v3(4.0f, 0.2f, 0.0f);
const f32 INVADER_TURNING_DISTANCE = (INVADER_MARGIN_X * INVADERS_PER_ROW) / 2;
const quantity<si::time, f32> INVADER_FIRE_INTERVAL = 5.0f * si::seconds;

const qv4 INVADER_ORIENTATION(0, 0, 0.707107f, -0.707107f);

bool isInvader(boost::shared_ptr<BFG::GameObject> go);

const s32 A_SHIP_AXIS_Y = 10000;
const s32 A_SHIP_FIRE   = 10001;
const s32 A_QUIT        = 10002;
const s32 A_FPS         = 10003;

const s32 A_CALL_REINFORCEMENTS = 11000;

class InvaderControl : public Property::Concept
{
public:
	InvaderControl(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "InvaderControl", pid),
	mLastSidestep(0),
	mDirection(1.0f, -1.0f, 0),
	mDistanceFromStart(0,0,0)
	{
		require("Physical");
	}

	void internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
	{
		Location go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);

		v3 delta = INVADER_VELOCITY * timeSinceLastFrame.value() * mDirection;
		go.position += delta;

		mDistanceFromStart.x += delta.x;

		checkPosition(go.position, mDirection);

		go.orientation = INVADER_ORIENTATION;

		emit<Physics::Event>(ID::PE_UPDATE_POSITION, go.position, ownerHandle());
		emit<Physics::Event>(ID::PE_UPDATE_ORIENTATION, go.orientation, ownerHandle());
	}

	void checkPosition(v3& position, v3& direction) const
	{
		// Move into current direction, if farer than wall, move opposite
		if (std::abs(mDistanceFromStart.x) > INVADER_TURNING_DISTANCE)
		{
			const f32 xSpeed = 1.0f;
			direction.x = xSpeed * -sign(mDistanceFromStart.x);
		}
	}
	
	quantity<si::time, f32> mLastSidestep;
	v3                      mDirection;
	v3                      mDistanceFromStart;
};

class ShipControl : public Property::Concept
{
public:
	ShipControl(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "ShipControl", pid)
	{
		require("Physical");
		requestEvent(ID::GOE_CONTROL_YAW);
	}
	
	void internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
	{
		Location go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);
	
		bool setPos = false;
	
		// Simulate a wall
		if (std::abs(go.position.x) > DISTANCE_TO_WALL)
		{
			emit<Physics::Event>(ID::PE_UPDATE_VELOCITY, v3::ZERO, ownerHandle());
			go.position.x = sign(go.position.x) * (DISTANCE_TO_WALL - 0.1f);
			setPos = true;
		}
	
		// Make sure it doesn't move too much on the z axis
		if (std::abs(go.position.z) - OBJECT_Z_POSITION > EPSILON_F)
		{
			go.position.z = OBJECT_Z_POSITION;
			setPos = true;
		}

		if (setPos)
			emit<Physics::Event>(ID::PE_UPDATE_POSITION, go.position, ownerHandle());
	}
	
	void internalOnEvent(EventIdT action,
	                     Property::Value payload,
	                     GameHandle module,
	                     GameHandle sender)
	{
		switch(action)
		{
			case ID::GOE_CONTROL_YAW:
			{
				// Make the ship tilt a bit when moving
				qv4 tilt;
				qv4 turn;
				fromAngleAxis(turn, -90.0f * DEG2RAD, v3::UNIT_X);
				fromAngleAxis(tilt, payload * -45.0f * DEG2RAD, v3::UNIT_Z);
				
				qv4 newOrientation = turn * tilt;
	
				// Move it left or right
				v3 newVelocity = v3(payload * SHIP_SPEED_MULTIPLIER,0,0);
				
				emit<Physics::Event>(ID::PE_UPDATE_ORIENTATION, newOrientation, ownerHandle());
				emit<Physics::Event>(ID::PE_UPDATE_VELOCITY, newVelocity, ownerHandle());
				break;
			}
		}
	}
};

class AssistanceControl : public Property::Concept
{
public:
	AssistanceControl(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "AssistanceControl", pid)
	{
		require("ShipControl");
	}
	
	void internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
	{
	}
	
	void internalOnEvent(EventIdT action,
	                     Property::Value payload,
	                     GameHandle module,
	                     GameHandle sender)
	{
		switch(action)
		{
		}
	}
};

class Cannon : public Property::Concept
{
public:
	Cannon(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "Cannon", pid),
	mAutoRocketAmmo(0)
	{
		require("Physical");

		initvar(ID_PROJECTILE_SPEED);
		initvar(ID_PROJECTILE_SPAWN_DISTANCE);

		requestEvent(ID::GOE_FIRE_ROCKET);
		requestEvent(ID::GOE_POWERUP);
}

	void internalOnEvent(EventIdT action,
	                     Property::Value payload,
	                     GameHandle module,
	                     GameHandle sender)
	{
		switch(action)
		{
			case ID::GOE_FIRE_ROCKET:
			{
				if (mAutoRocketAmmo > 0)
				{
					fireRocket(true);
					--mAutoRocketAmmo;
				}
				else
				{
					fireRocket(false);
				}
				break;
			}
			case ID::GOE_POWERUP:
			{
				s32 newAmmo = payload;
				mAutoRocketAmmo += newAmmo;
				break;
			}
		}
	}

	void fireRocket(bool autoRocket) const
	{
		std::vector<GameHandle> targets = environment()->find_all(isInvader);

		if (autoRocket && targets.empty())
			return;
		
		const Location& go = getGoValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);

		const f32& projectileSpeed = value<f32>(ID_PROJECTILE_SPEED, rootModule());
		const f32& projectileSpawnDistance = value<f32>(ID_PROJECTILE_SPAWN_DISTANCE, rootModule());

		Location spawnLocation;
		spawnLocation.position = go.position + go.orientation.zAxis() * projectileSpawnDistance;
		spawnLocation.orientation = go.orientation;

		// Make Name
		static size_t count = 0;
		++count;
		std::stringstream ss;
		ss << "Projectile " << count;

		// Make Configuration
		Loader::ObjectParameter op;
		op.mHandle = generateHandle();
		op.mName = ss.str();
		if (autoRocket) op.mType = "AutoProjectile";
		else            op.mType = "Projectile";
		op.mLocation = spawnLocation;
		op.mLinearVelocity = v3(projectileSpeed) * go.orientation.zAxis();
		
		emit<SectorEvent>(ID::S_CREATE_GO, op);

		playLaserSound();

		if (autoRocket)
		{
			GameHandle randomInvader = targets[rand()%targets.size()];
			emit<GameObjectEvent>(ID::GOE_AUTONAVIGATE, randomInvader, op.mHandle);
		}
	}
	
	void playLaserSound() const
	{
		GameHandle audioObjectHandle = generateHandle();
		Path path;
		AOCreation ao(audioObjectHandle, path.Get(ID::P_SOUND_EFFECTS) + "Laser_003.wav");

		emit<Audio::AudioEvent>(ID::AE_CREATE_AUDIO_OBJECT, ao);
		emit<Audio::AudioEvent>(ID::AE_PLAY, true, audioObjectHandle);
	}

	s32 mAutoRocketAmmo;
};

class Collectable : public Property::Concept
{
public:
	Collectable(GameObject& Owner, BFG::PluginId pid) :
	Property::Concept(Owner, "Collectable", pid),
	mUsed(false)
	{
		require("Physical");
		require("Destroyable");

		requestEvent(ID::PE_CONTACT);
	}
	
	void internalOnEvent(EventIdT action,
	                     Property::Value payload,
	                     GameHandle module,
	                     GameHandle sender)
	{
		// Ignore additional PE_CONTACT events
		if (mUsed)
			return;
	
		switch(action)
		{
			case ID::PE_CONTACT:
			{
				onContact(sender);
				break;
			}
		}
	}

	void onContact(GameHandle sender)
	{
		mUsed = true;
		
		s32 probabilityReinforcementsInPercent = 25;
		if (rand()%100 > probabilityReinforcementsInPercent)
		{
			emit<Controller_::VipEvent>(A_CALL_REINFORCEMENTS, 0);
		}
		else
		{
			// Very Cannon specific stuff.
			s32 ammo = 5;
			emit<GameObjectEvent>(ID::GOE_POWERUP, ammo, sender);
		}
	}

	bool mUsed;
};
	
	
class SiConceptFactory : public Property::ConceptFactory
{
public:
	SiConceptFactory(Property::PluginId pid) :
	ConceptFactory(pid)
	{}

	virtual boost::shared_ptr<Property::Concept>
	createConcept(Property::ConceptId pc, GameObject& go)
	{
		PROPERTYCONCEPT_BUILD_LIST_BEGIN
		PROPERTYCONCEPT_CASE(ShipControl)
		PROPERTYCONCEPT_CASE(InvaderControl)
		PROPERTYCONCEPT_CASE(AssistanceControl)
		PROPERTYCONCEPT_CASE(Cannon)
		PROPERTYCONCEPT_CASE(Collectable)
		PROPERTYCONCEPT_BUILD_LIST_END
	}
};

struct SiSerialiser : BFG::Property::Serialiser
{
	virtual ~SiSerialiser() {}

	//! \brief VarId to String serialisation
	//! \param[in] in VarId which shall be serialised to a String
	//! \param[out] out Result
	//! \return true if serialisation was successful, false otherwise
	virtual bool varToString(BFG::Property::ValueId::VarIdT in,
	                         std::string& out) const
	{
		std::cout << "STUB: SiSerialiser::varToString [" << in << "]" << std::endl;
		return false;
	}

	//! \brief String to VarId serialisation
	//! \param[in] in String which shall be serialised to a VarId
	//! \param[out] out Result
	//! \return true if serialisation was successful, false otherwise
	virtual bool stringToVar(const std::string& in,
	                         BFG::Property::ValueId::VarIdT& out) const
	{
		if (in == "ProjectileSpeed")
		{
			out = ID_PROJECTILE_SPEED;
			return true;
		}
		else if (in == "ProjectileSpawnDistance")
		{
			out = ID_PROJECTILE_SPAWN_DISTANCE;
			return true;
		}
		return false;
	}
};

// ------------------------ SPACE PLUGIN -----------------------------

struct SiPlugin : BFG::Property::Plugin
{
	SiPlugin(PluginId pluginId) : 
	Plugin(pluginId, "Space Invader Properties")
	{
		addConcept("ShipControl");
		addConcept("InvaderControl");
		addConcept("Cannon");
		addConcept("Collectable");
	}

	virtual ~SiPlugin() {}

	virtual boost::shared_ptr<BFG::Property::ConceptFactory>
	conceptFactory() const
	{
		boost::shared_ptr<Property::ConceptFactory> f(new SiConceptFactory(id()));
		return f;
	}

	virtual boost::shared_ptr<BFG::Property::Serialiser>
	serialiser() const
	{
		boost::shared_ptr<Property::Serialiser> s(new SiSerialiser);
		return s;
	}
};

bool nearestToPlayer(boost::shared_ptr<BFG::GameObject> go,
                     std::pair<GameHandle, f32>& bestCandidate,
                     const f32& minDistanceToPlayer,
                     const v3& playerPosition)
{
	if (! go->satisfiesRequirement("InvaderControl"))
		return false;

	const Location& invader = go->getValue<Location>(ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);
	
	f32 invaderPlayerDistance = BFG::distance(invader.position, playerPosition);
	
	if (invaderPlayerDistance < bestCandidate.second &&
	    invaderPlayerDistance > minDistanceToPlayer)
		bestCandidate = std::make_pair(go->getHandle(), invaderPlayerDistance);

	// We need to iterate through all invaders
	return false;
}

bool isPlayer(boost::shared_ptr<BFG::GameObject> go)
{
	return go->satisfiesRequirement("ShipControl");
}

bool isInvader(boost::shared_ptr<BFG::GameObject> go)
{
	return go->satisfiesRequirement("InvaderControl");
}

struct InvaderGeneral : Emitter
{
	InvaderGeneral(EventLoop* loop,
	               boost::shared_ptr<BFG::Environment> environment) :
	Emitter(loop),
	mEnvironment(environment),
	mLastShot(0),
	mWaveCount(0)
	{
		spawnWave();
		++mWaveCount;
	}
	
	void spawnWave()
	{
		Loader::ObjectParameter op;

		for (size_t i=0; i < (size_t) INVADERS_PER_ROW; ++i)
		{
			for (size_t j=0; j < (size_t) INVADERS_PER_COL; ++j)
			{
				std::stringstream ss;
				ss << "Invader No. X:" << i << " Y:" << j;

				boost::shared_ptr<GameObject> invader;
			
				op = Loader::ObjectParameter();
				op.mHandle = generateHandle();
				op.mName = ss.str();
				op.mType = "Invader";
				op.mLocation.position = v3
				(
					INVADER_MARGIN_X*i - (INVADERS_PER_ROW*INVADER_MARGIN_X)/2,
					INVADER_MARGIN_Y*j - (INVADERS_PER_COL*INVADER_MARGIN_Y)/2 + INVADER_BOTTOM_MARGIN,
					OBJECT_Z_POSITION
				);
				op.mLocation.orientation = INVADER_ORIENTATION;
				op.mLinearVelocity = v3::ZERO;

				emit<SectorEvent>(ID::S_CREATE_GO, op);
			}
		}
	}

	void update(quantity<si::time, f32> timeSinceLastFrame)
	{
		mLastShot += timeSinceLastFrame;
		
		if (mLastShot < INVADER_FIRE_INTERVAL)
			return;

		if (mEnvironment->find(isInvader) == NULL_HANDLE)
		{
			spawnWave();
			++mWaveCount;
			return;
		}

		GameHandle player = mEnvironment->find(isPlayer);
		
		// Player dead?
		if (player == NULL_HANDLE)
			return;
		
		const Location& playerLoc = mEnvironment->getGoValue<Location>(player, ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);
		
		f32 lastPlayerInvaderDistance = 0;
		for (size_t i=0; i < mWaveCount; ++i)
		{
			// Handle of Invader and its Distance to the Player
			std::pair<GameHandle, f32> bestCandidate(NULL_HANDLE, 999999.9f);

			GameHandle dummy = mEnvironment->find
			(
				boost::bind
				(
					nearestToPlayer,
					_1,
					boost::ref(bestCandidate),
					boost::ref(lastPlayerInvaderDistance),
					boost::ref(playerLoc.position)
				)
			);
			
			emit<GameObjectEvent>(ID::GOE_FIRE_ROCKET, 0, bestCandidate.first);
			
			lastPlayerInvaderDistance = bestCandidate.second + 0.1f;
		}
		
		mLastShot = 0;
	}
	
	boost::shared_ptr<BFG::Environment> mEnvironment;
	quantity<si::time, f32>             mLastShot;
	size_t                              mWaveCount;
};

struct HumanGeneral : Emitter
{
	HumanGeneral(EventLoop* loop,
	             boost::shared_ptr<BFG::Environment> environment) :
	Emitter(loop),
	mEnvironment(environment),
	mLastPowerupSpawned(0 * si::seconds)
	{
		loop->connect(A_CALL_REINFORCEMENTS, this, &HumanGeneral::EventHandler);
	}
	
	void update(quantity<si::time, f32> timeSinceLastFrame)
	{
		mLastPowerupSpawned += timeSinceLastFrame;
		
		if (mLastPowerupSpawned > 15.0f * si::seconds)
		{
			spawnPowerupAtRandomPosition();
			mLastPowerupSpawned = 0;
		}
	}

	void EventHandler(Controller_::VipEvent* iCE)
	{
		switch (iCE->getId())
		{
		case A_CALL_REINFORCEMENTS:
			callReinforcements();
			break;
		}
	}

	void callReinforcements() const
	{
		std::vector<GameHandle> targets = mEnvironment->find_all(isInvader);
		
		float yPos = -10.0f; 
		for (size_t i=0; i<20; ++i)
		{
			Loader::ObjectParameter op;
			op.mHandle = generateHandle();
			op.mName = "Reinforcement";
			op.mType = "AutoProjectile";
			op.mLocation = v3(100.0f + yPos, yPos, OBJECT_Z_POSITION - 50.0f); // - 5.0f); // + SPECIAL_PACKER_MESH_OFFSET);
			fromAngleAxis(op.mLocation.orientation, -90 * DEG2RAD, v3::UNIT_X);
			emit<SectorEvent>(ID::S_CREATE_GO, op);
			yPos += 5.0f;

			GameHandle randomInvader = targets[rand()%targets.size()];
			emit<GameObjectEvent>(ID::GOE_AUTONAVIGATE, randomInvader, op.mHandle);
		}
	}
	
	void spawnPowerupAtRandomPosition() const
	{
		Loader::ObjectParameter op;
		op = Loader::ObjectParameter();
		op.mHandle = generateHandle();
		op.mName = "Test Powerup";
		op.mType = "Powerup";
		op.mAngularVelocity = v3(0.0f, 5.0f, 0.0f);

		int spawnRange = static_cast<int>(DISTANCE_TO_WALL) * 2;
		int spawnPos = rand()%spawnRange - DISTANCE_TO_WALL;

		op.mLocation = v3
		(
			static_cast<f32>(spawnPos),
			-NEGATIVE_SHIP_Y_POSITION,
			OBJECT_Z_POSITION
		);
		emit<SectorEvent>(ID::S_CREATE_GO, op);
	}
	
	boost::shared_ptr<BFG::Environment> mEnvironment;	
	quantity<si::time, f32>             mLastPowerupSpawned;
};

struct MainState : Emitter
{
	MainState(GameHandle handle, EventLoop* loop) :
	Emitter(loop),
	mPlayer(NULL_HANDLE),
	mEnvironment(new Environment),
	mClock(new Clock::StopWatch(Clock::milliSecond)),
	mExitNextTick(false),
	mInvaderGeneral(loop, mEnvironment),
	mHumanGeneral(loop, mEnvironment)
	{
		Path p;
		std::string level = p.Get(ID::P_SCRIPTS_LEVELS) + "spaceinvaders/";
		std::string def = p.Get(ID::P_SCRIPTS_LEVELS) + "default/";

		Loader::LevelConfig lc;

		lc.mModules.push_back(def + "Object.xml");
		lc.mAdapters.push_back(def + "Adapter.xml");
		lc.mConcepts.push_back(def + "Concept.xml");
		lc.mProperties.push_back(def + "Value.xml");

		lc.mModules.push_back(level + "Object.xml");
		lc.mAdapters.push_back(level + "Adapter.xml");
		lc.mConcepts.push_back(level + "Concept.xml");
		lc.mProperties.push_back(level + "Value.xml");

		using BFG::Property::ValueId;

		PluginId spId = ValueId::ENGINE_PLUGIN_ID;
		PluginId sipId = BFG::Property::generatePluginId<PluginId>();

		boost::shared_ptr<BFG::SpacePlugin> sp(new BFG::SpacePlugin(spId));
		boost::shared_ptr<SiPlugin> sip(new SiPlugin(sipId));

		mPluginMap.insert(sp);
		mPluginMap.insert(sip);

		boost::shared_ptr<Loader::Interpreter> interpreter(new Loader::Interpreter(mPluginMap));

		boost::shared_ptr<Loader::GameObjectFactory> gof;
		gof.reset(new Loader::GameObjectFactory(loop, lc, mPluginMap, interpreter, mEnvironment, handle));

		mSector.reset(new Sector(loop, 1, "Blah", gof));
		
		View::SkyCreation sc("sky01");
		emit<View::Event>(ID::VE_SET_SKY, sc);

		Loader::ObjectParameter op;
		op.mHandle = generateHandle();
		op.mName = "The Hero's Mighty Ship";
		op.mType = "Ship";
		op.mLocation = v3(0.0f, -NEGATIVE_SHIP_Y_POSITION, OBJECT_Z_POSITION); // - 5.0f); // + SPECIAL_PACKER_MESH_OFFSET);
		fromAngleAxis(op.mLocation.orientation, -90 * DEG2RAD, v3::UNIT_X);
		boost::shared_ptr<GameObject> playerShip = gof->createGameObject(op);
		mSector->addObject(playerShip);
		
		mPlayer = playerShip->getHandle();

		// \Hack This level is a static list at Audio::DummyLoader.. must be removed in advance of AudioBlaster development.
		emit<Audio::AudioEvent>(ID::AE_SET_LEVEL_CURRENT, std::string("DummyLevel1"));

		mClock->start();
	}

	void ControllerEventHandler(Controller_::VipEvent* iCE)
	{
		switch(iCE->getId())
		{
			case A_SHIP_AXIS_Y:
				emit<GameObjectEvent>(ID::GOE_CONTROL_YAW, boost::get<float>(iCE->getData()), mPlayer);
				break;

			case A_SHIP_FIRE:
			{
				emit<GameObjectEvent>(ID::GOE_FIRE_ROCKET, 0, mPlayer);
				break;
			}
			
			case A_QUIT:
			{
				mExitNextTick = true;
				emit<BFG::View::Event>(BFG::ID::VE_SHUTDOWN, 0);
				break;
			}
			
			case A_FPS:
			{
 				emit<BFG::View::Event>(BFG::ID::VE_DEBUG_FPS, boost::get<bool>(iCE->getData()));
				break;
			}
		}
	}

	void LoopEventHandler(LoopEvent* iLE)
	{
		if (mExitNextTick)
		{
			// Error happened, while doing stuff
			iLE->getData().getLoop()->setExitFlag();
		}

		long timeSinceLastFrame = mClock->stop();
		if (timeSinceLastFrame)
			mClock->start();

		f32 timeInSeconds = static_cast<f32>(timeSinceLastFrame) / Clock::milliSecond;
		tick(timeInSeconds);
	}
		
	void tick(const f32 timeSinceLastFrame)
	{
		if (timeSinceLastFrame < EPSILON_F)
			return;

		quantity<si::time, f32> TSLF = timeSinceLastFrame * si::seconds;

		mSector->update(TSLF);
		mInvaderGeneral.update(TSLF);
		mHumanGeneral.update(TSLF);

		emit<Physics::Event>(ID::PE_STEP, TSLF.value());
	}

	GameHandle mPlayer;

	boost::shared_ptr<Sector> mSector;
	boost::shared_ptr<Environment> mEnvironment;
	
	boost::scoped_ptr<Clock::StopWatch> mClock;
	
	InvaderGeneral mInvaderGeneral;
	HumanGeneral   mHumanGeneral;
	
	BFG::Property::PluginMapT mPluginMap;
	
	bool mExitNextTick;
};

void fireParticles(Ogre::ParticleSystem* particleSystem,
				   Ogre::SceneNode* sceneNode,
				   f32 startTime,
				   f32 endTime,
				   f32 elapsedTime)
{
	if ((elapsedTime > startTime) && (elapsedTime < endTime))
	{
		if(!particleSystem->isAttached())
			sceneNode->attachObject(particleSystem);
		return;
	}
	if (elapsedTime > endTime)
	{
		if (particleSystem->isAttached())
			sceneNode->detachObject(particleSystem);	
		return;
	}
}

class PowerupEffect : public View::Effect
{
public:
	PowerupEffect(const v3& position, f32 intensity) :
	mPosition(position),
	mIntensity(intensity),
	mHandle(generateHandle()),
	mNode(NULL),
	mParticleSystem(NULL),
	mElapsedTime(0.0f)
	{
		Ogre::SceneManager* sceneMgr =
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		mNode = static_cast<Ogre::SceneNode*>
			(sceneMgr->getRootSceneNode()->createChild(stringify(mHandle)));

		mNode->setPosition(Ogre::Vector3(mPosition.ptr()));

		mParticleSystem = sceneMgr->createParticleSystem
		(
			stringify(mHandle) + "particle",
			"Powerup_1"
		);

		if (!mParticleSystem)
			throw std::runtime_error("Explosion: creating mParticleSystem failed!");
	}

	~PowerupEffect()
	{
		Ogre::SceneManager* sceneMgr =
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		sceneMgr->destroyParticleSystem(mParticleSystem);
		sceneMgr->destroySceneNode(mNode);
	}

	bool frameStarted( const Ogre::FrameEvent& evt )
	{
		if (done())
			return true;

		mElapsedTime += evt.timeSinceLastFrame;

		fireParticles
		(
			mParticleSystem,
			mNode,
			0.0f,
			2.0f,
			mElapsedTime
		);

		if (mElapsedTime > 2.0f)
			finished();

		return true;
	}

private:
	v3 mPosition;
	f32 mIntensity;
	GameHandle mHandle;
	Ogre::SceneNode* mNode;
	Ogre::ParticleSystem* mParticleSystem;
	f32 mElapsedTime;
};

struct ViewMainState : public View::State
{
public:
	ViewMainState(GameHandle handle, EventLoop* loop) :
	State(handle, loop)
	{
		registerEventHandler();
	}

	~ViewMainState()
	{
		unregisterEventHandler();
	}

	void viewEventHandler(View::Event* VE)
	{
		switch(VE->getId())
		{
		case ID::VE_EFFECT:
			onEffect(boost::get<View::EffectCreation>(VE->getData()));
			break;
		default:
			throw std::logic_error("ViewMainState::eventHandler: received unhandled event!");
		}
	}

	bool frameStarted(const Ogre::FrameEvent& evt)
	{
		erase_if(mEffects, boost::bind(&View::Effect::done, _1));
		return true;
	}

	bool frameRenderingQueued(const Ogre::FrameEvent& evt)
	{
		return true;
	}

	bool frameEnded(const Ogre::FrameEvent& evt)
	{
		return true;
	}

private:
	void registerEventHandler()
	{
		View::Main::eventLoop()->connect(ID::VE_EFFECT, this, &ViewMainState::viewEventHandler);
	}
	void unregisterEventHandler()
	{
		View::Main::eventLoop()->disconnect(ID::VE_EFFECT, this);
	}
	
	void onEffect(const View::EffectCreation& ec)
	{
		const CharArray128T& ca = ec.get<0>();
		const v3& position = ec.get<1>();
		const f32 intensity = ec.get<2>();
		const std::string effect(ca.data());
	
		boost::shared_ptr<View::Effect> e;
		if (effect == "Explosion_1")
		{
			e.reset(new View::Explosion(position, intensity));
			mEffects.push_back(e);
		}
		else if(effect == "Explosion_2")
		{
 			e.reset(new View::Explosion2(position, intensity));
			mEffects.push_back(e);
		}
		else if (effect == "Powerup_1")
		{
			e.reset(new PowerupEffect(position, intensity));
			mEffects.push_back(e);
		}
		else
			errlog << "ViewMainState::onEffect(): Got effect (\""
			       << effect
			       << "\")";
	}

	virtual void pause(){ registerEventHandler(); }
	virtual void resume(){ unregisterEventHandler(); }
	
	std::vector<boost::shared_ptr<View::Effect> > mEffects;
};

// This is the Ex-'GameStateManager::SingleThreadEntryPoint(void*)' function
void* SingleThreadEntryPoint(void *iPointer)
{
	EventLoop* loop = static_cast<EventLoop*>(iPointer);
	
	GameHandle siHandle = BFG::generateHandle();
	
	// Hack: Using leaking pointers, because vars would go out of scope
	MainState* ps = new MainState(siHandle, loop);
	ViewMainState* vps = new ViewMainState(siHandle, loop);

	// Init Controller
	GameHandle handle = generateHandle();

	{
		BFG::Controller_::ActionMapT actions;
		actions[A_SHIP_AXIS_Y] = "A_SHIP_AXIS_Y";
		actions[A_SHIP_FIRE]   = "A_SHIP_FIRE";
		actions[A_QUIT]        = "A_QUIT";
		actions[A_FPS]         = "A_FPS";
		BFG::Controller_::sendActionsToController(loop, actions);
	
		Path path;
		const std::string config_path = path.Expand("SpaceInvaders.xml");
		const std::string state_name = "SpaceInvaders";
		
		Controller_::StateInsertion si(config_path, state_name, handle, true);

		EventFactory::Create<Controller_::ControlEvent>
		(
			loop,
			ID::CE_LOAD_STATE,
			si
		);

		loop->connect(A_SHIP_AXIS_Y, ps, &MainState::ControllerEventHandler);
		loop->connect(A_SHIP_FIRE, ps, &MainState::ControllerEventHandler);
		loop->connect(A_QUIT, ps, &MainState::ControllerEventHandler);
		loop->connect(A_FPS, ps, &MainState::ControllerEventHandler);
	}

	// Init AudioBlaster
	Audio::AudioBlaster* audioBlaster = new Audio::AudioBlaster();

	assert(loop);
	loop->registerLoopEventListener(ps, &MainState::LoopEventHandler);
	return 0;
}

int main( int argc, const char* argv[] ) try
{
	srand(time(NULL));

#if defined(_DEBUG) || !defined(NDEBUG)
	Base::Logger::Init(Base::Logger::SL_DEBUG, "Logs/Si.log");
#else
	Base::Logger::Init(Base::Logger::SL_INFORMATION, "Logs/Si.log");
#endif

	EventLoop iLoop(true);

	size_t controllerFrequency = 1000;

	const std::string caption = "Engine Test 02: Space Invaders";

	// Hack: Actually, EventLoop should take ownership over an entry point
	//       pointer, but it doesn't yet. So here comes the workaround:
	boost::scoped_ptr<Base::IEntryPoint> epView(View::Interface::getEntryPoint(caption));

	iLoop.addEntryPoint(epView.get());
	iLoop.addEntryPoint(ModelInterface::getEntryPoint());
	iLoop.addEntryPoint(ControllerInterface::getEntryPoint(controllerFrequency));
	iLoop.addEntryPoint(Physics::Interface::getEntryPoint());
	iLoop.addEntryPoint(Audio::AudioInterface::getEntryPoint());
	iLoop.addEntryPoint(new Base::CEntryPoint(SingleThreadEntryPoint));

	iLoop.run();
}
catch (Ogre::Exception& e)
{
	showException(e.getFullDescription().c_str());
}
catch (std::exception& ex)
{
	showException(ex.what());
}
catch (...)
{
	showException("Unknown exception");
}
