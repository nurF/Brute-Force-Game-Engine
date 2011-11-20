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

#ifndef AUDIO_BLASTER_H
#define AUDIO_BLASTER_H

#include <map>
#include <boost/shared_ptr.hpp>

#include <al.h>
#include <alc.h>

#include <EventSystem/Core/EventLoop.h>

#include <AudioBlaster/Defines.h>
#include <AudioBlaster/AudioEvent.h>


namespace BFG {
namespace Audio {

struct Level;

class AUDIO_BLASTER_API AudioBlaster
{
public:
	AudioBlaster();
	~AudioBlaster();

	void deleteAudioObject(GameHandle);

private:
	void init();

	void initLevels();
	void unloadLevel(const std::string& levelName);
	void setLevelCurrent(const std::string& levelName);

	void eventHandler(AudioEvent* AE);
	void loopEventHandler(LoopEvent* loopEvent);

	void onSetLevelCurrent(const AudioPayloadT& payload);
	void onVelocityPlayer(const AudioPayloadT& payload);
	void onOrientationPlayer(const AudioPayloadT& payload);
	void onEventMasterGain(const AudioPayloadT& payload);
	void onEventPositionPlayer(const AudioPayloadT& payload);
	void onEventCreateAudioObject(const AudioPayloadT& payload);

	ALCdevice* mDevice;
	ALCcontext* mContext;

	typedef std::map<std::string, boost::shared_ptr<Level> > LevelMapT;
	LevelMapT mLevels;
	boost::shared_ptr<Level> mCurrentLevel;
};

} // namespace AudioBlaster
} // namespace BFG

#endif