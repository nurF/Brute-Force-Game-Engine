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

#ifndef AUDIO_BLASTER_WAVE_FILE_H
#define AUDIO_BLASTER_WAVE_FILE_H

#include <string>
#include <boost/scoped_array.hpp>
#include <al.h>
#include <sndfile.h>

#include <AudioBlaster/Defines.h>
#include <AudioBlaster/FileBase.h>

namespace BFG {
namespace Audio {

class AUDIO_BLASTER_API WaveFile : public AudioFile
{

public:
	WaveFile(const std::string& filename);
	~WaveFile();

	void read(ALuint bufferID);

private:

	void close();
	void open(const std::string& filename);

	SNDFILE* mSoundFile;
	boost::scoped_array<int> mBuffer;
	const unsigned int mBUFFER_SIZE;
	ALenum mFormat;
};

} // namespace AudioBlaster
} // namespace BFG

#endif