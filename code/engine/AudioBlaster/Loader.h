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

#ifndef AUDIO_BLASTER_LOADER_H
#define AUDIO_BLASTER_LOADER_H

#include <string>
#include <vector>
#include <Core/Path.h>

namespace BFG {
namespace Audio {

//! This is just a test-dummy! Replace this with serious code...
class DummyLoader
{
public:

	typedef std::pair<std::string, std::vector <std::string> > Level;

	std::vector<Level> loadAudioConfig()
	{
		Path path;
		std::vector<Level> levels;

		std::vector<std::string> fileNames1;

		fileNames1.push_back(path.Get(ID::P_SOUND_MUSIC)+"02_Deimos - Flottenkommando.ogg");
		fileNames1.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_003.wav");
		fileNames1.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Destruction_ExplosionD9.wav");
		fileNames1.push_back(path.Get(ID::P_SOUND_EFFECTS)+"Laser_008.wav");

		Level level1 = std::make_pair("DummyLevel1", fileNames1);
		levels.push_back(level1);

		std::vector<std::string> fileNames2;
		fileNames2.push_back(path.Get(ID::P_SOUND_MUSIC)+"6 Fleet's Arrival.ogg");
		fileNames2.push_back(path.Get(ID::P_SOUND_MUSIC)+"05_Deimos - Wild Space.ogg");

		Level level2 = std::make_pair("DummyLevel2", fileNames2);
		levels.push_back(level2);

		return levels;
	}
};

} // namespace AudioBlaster
} // namespace BFG

#endif