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

#ifndef AUDIO_BLASTER_BASE_FILE_H
#define AUDIO_BLASTER_BASE_FILE_H

#include <string>
#include <al.h>

#include <AudioBlaster/Defines.h>

namespace BFG {
namespace Audio {

//! This is an Interface for reading AudioData out of several file formats.
class AUDIO_BLASTER_API AudioFile
{
public:
 	virtual ~AudioFile() {}

 	//! The function gets one OpenAl bufferId whose buffer will be filled with audio data.
 	//! So every call of read fills one buffer. The size of one buffer depends on the sample-rate. 
 	virtual void read(ALuint bufferId) = 0;

private:
 	virtual void open(const std::string& filename) = 0;
 	virtual void close() = 0;
};

} // namespace AudioBlaster
} // namespace BFG

#endif