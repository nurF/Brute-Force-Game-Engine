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

#include <AudioBlaster/AudioBlaster.h>

#include <Base/CLogger.h>
#include <EventSystem/Core/EventLoop.h>

#include <AudioBlaster/Enums.hh>
#include <AudioBlaster/Main.h>
#include <AudioBlaster/Transport.h>
#include <AudioBlaster/Level.h>
#include <AudioBlaster/AudioObject.h>
#include <AudioBlaster/Loader.h>
#include <AudioBlaster/HelperFunctions.h>


namespace BFG {
namespace Audio {

AudioBlaster::AudioBlaster():
	mDevice(NULL),
	mContext(NULL)
{
	mCurrentLevel.reset();
	init();
}

AudioBlaster::~AudioBlaster()
{
	AudioMain::eventLoop()->disconnect(ID::AE_MASTER_GAIN, this);
	AudioMain::eventLoop()->disconnect(ID::AE_POSITION_PLAYER, this);
	AudioMain::eventLoop()->disconnect(ID::AE_CREATE_AUDIO_OBJECT, this);
	AudioMain::eventLoop()->disconnect(ID::AE_ORIENTATION_PLAYER, this);
	AudioMain::eventLoop()->disconnect(ID::AE_VELOCITY_PLAYER, this);
	AudioMain::eventLoop()->disconnect(ID::AE_SET_LEVEL_CURRENT, this);

	mLevels.clear();

	alcDestroyContext(mContext);
	alcCloseDevice(mDevice);

	mContext = NULL;
	mDevice = NULL;

	AudioMain::eventLoop()->setExitFlag(true);
	AudioMain::eventLoop()->doLoop();
}

void AudioBlaster::init()
{
	ALenum errorCode = AL_NONE;

	mDevice = alcOpenDevice(0); // 0 is default device.

	if (!mDevice)
	{
		throw std::logic_error("Open audio device failed!");
	}
	else
		infolog << "OpenAl device created";

	alListener3f(AL_POSITION, 0, 0, 0);
	alListener3f(AL_ORIENTATION, 0, 0, 1);
	alListener3f(AL_VELOCITY, 0, 0, 0);

	mContext = alcCreateContext(mDevice, 0);

	if (!mContext)
		throw std::logic_error("Creation of AlContext for level failed! ALerror: "+stringifyAlError(alGetError()));
	else
		infolog << "Al context created";

	if (!alcMakeContextCurrent(mContext))
		throw std::logic_error("Making alContext current failed at Level::load. ALError: "+stringifyAlError(alGetError()));
	else
		infolog << "Set Al context current.";

	AudioMain::eventLoop()->connect(ID::AE_MASTER_GAIN, this, &AudioBlaster::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_POSITION_PLAYER, this, &AudioBlaster::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_CREATE_AUDIO_OBJECT, this, &AudioBlaster::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_ORIENTATION_PLAYER, this, &AudioBlaster::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_VELOCITY_PLAYER, this, &AudioBlaster::eventHandler);
	AudioMain::eventLoop()->connect(ID::AE_SET_LEVEL_CURRENT, this, &AudioBlaster::eventHandler);

	initLevels();
}

void AudioBlaster::initLevels()
{
	DummyLoader loader;
	std::vector<DummyLoader::Level> levels = loader.loadAudioConfig();

	std::vector<DummyLoader::Level>::iterator it = levels.begin();

	for(; it != levels.end(); ++it)
	{
		DummyLoader::Level temp = *it;

		boost::shared_ptr<Level> newLevel;
		newLevel.reset(new Level(temp.second));
		mLevels.insert(std::make_pair(temp.first, newLevel));
	}
}

void AudioBlaster::unloadLevel(const std::string& levelName)
{
	LevelMapT::iterator it = mLevels.find(levelName);

	if (it == mLevels.end())
		return;

	mLevels.erase(it);
}

void AudioBlaster::setLevelCurrent(const std::string& levelName)
{
	LevelMapT::iterator it = mLevels.find(levelName);

	if (it == mLevels.end())
		throw std::logic_error("AudioBlaster: Level: "+ levelName +" doesn't exists!");

	mCurrentLevel = it->second;
	mCurrentLevel->load();
}

void AudioBlaster::eventHandler(AudioEvent* AE)
{
	switch (AE->getId())
	{
	case ID::AE_MASTER_GAIN:
		onEventMasterGain(AE->getData());
		break;
	case ID::AE_POSITION_PLAYER:
		onEventPositionPlayer(AE->getData());
		break;
	case ID::AE_ORIENTATION_PLAYER:
		onOrientationPlayer(AE->getData());
		break;
	case ID::AE_VELOCITY_PLAYER:
		onVelocityPlayer(AE->getData());
		break;
	case ID::AE_CREATE_AUDIO_OBJECT:
		onEventCreateAudioObject(AE->getData());
		break;
	case ID::AE_SET_LEVEL_CURRENT:
		onSetLevelCurrent(AE->getData());
		break;
	default:
		throw std::logic_error("Unhandled event!");
	}
}

void AudioBlaster::onSetLevelCurrent(const AudioPayloadT& payload)
{
	std::string level = boost::get<std::string>(payload);
	setLevelCurrent(level);
}

void AudioBlaster::onVelocityPlayer(const AudioPayloadT& payload)
{
	v3 velocity = boost::get<v3>(payload);
	alListener3f(AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

void AudioBlaster::onOrientationPlayer(const AudioPayloadT& payload)
{
	v3 orientation = boost::get<v3>(payload);
	alListener3f(AL_ORIENTATION, orientation.x, orientation.y, orientation.z);
}

void AudioBlaster::onEventMasterGain(const AudioPayloadT& payload)
{
	ALfloat gain = static_cast<ALfloat>(boost::get<float>(payload));
	alListenerf(AL_GAIN, gain);
}

void AudioBlaster::onEventPositionPlayer(const AudioPayloadT& payload)
{
	v3 position = boost::get<v3>(payload);
	alListener3f(AL_POSITION, position.x, position.y, position.z);
}

void AudioBlaster::onEventCreateAudioObject(const AudioPayloadT& payload)
{
	AOCreation aoc = boost::get<AOCreation>(payload);

	mCurrentLevel->mObjects[aoc.mHandle] = boost::shared_ptr<AudioObject>
	(
		mCurrentLevel->mObjectFactory->createAudioObject(aoc, mCurrentLevel->mStreamWatch, *this)
	);
}

void AudioBlaster::loopEventHandler(LoopEvent* loopEvent)
{
	boost::this_thread::sleep(boost::posix_time::millisec(2));
}

void AudioBlaster::deleteAudioObject(GameHandle handle)
{
	Level::ObjectMapT::iterator it;
	it = mCurrentLevel->mObjects.find(handle);

	if (it != mCurrentLevel->mObjects.end())
	{
		mCurrentLevel->mObjects.erase(it);
	}
}

} // namespace AudioBlaster
} // namespace BFG
