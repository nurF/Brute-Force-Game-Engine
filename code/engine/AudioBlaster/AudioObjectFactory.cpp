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

#include <AudioBlaster/AudioObjectFactory.h>
#include <AudioBlaster/AudioBlaster.h>

namespace BFG {
namespace Audio {

AudioObjectFactory::AudioObjectFactory()
{
	loadPresets();
}

boost::shared_ptr<AudioObject> AudioObjectFactory::createAudioObject(AOCreation aoc, 
																	 boost::shared_ptr<StreamWatch> streamWatch,
																	 AudioBlaster& audioBlaster)
{
	bool updateable = false;

	if(aoc.mAudioObjectType == ID::AOT_PERMANENT)
		updateable = true;

	std::map<ID::AudioPreset, AOPresetData>::iterator it = mPresets.find(aoc.mAudioPreset);

	if (it == mPresets.end())
		std::logic_error("Fatal error at AudioObjectFactory. Unknown AudioPresetData type.");

	AOPresetData aop = it->second;

	return boost::shared_ptr<AudioObject>(new AudioObject(aoc.mHandle, 
	                                                      aoc.mName, 
														  streamWatch, 
														  aop, 
														  aoc.mLocation,
														  audioBlaster,
														  updateable));
}

void AudioObjectFactory::loadPresets()
{
	// Hack! will be substituted with xml loading mechanism later.

	mPresets.insert(std::make_pair(ID::AOP_DEFAULT_SOUND, AOPresetData(1.0f, 1.0f, 0.5f, 0.5f, 0.7f, 5.0f, 0.8f)));
}

AudioObjectFactory::~AudioObjectFactory()
{
	mPresets.clear();
}

} // namespace AudioBlaster
} // namespace BFG
