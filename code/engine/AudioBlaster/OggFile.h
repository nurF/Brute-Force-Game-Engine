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

#ifndef AUDIO_BLASTER_OGG_FILE_H
#define AUDIO_BLASTER_OGG_FILE_H

#include <string>
#include <boost/scoped_array.hpp>
#include <al.h>

#include <AudioBlaster/Defines.h>
#include <AudioBlaster/FileBase.h>

struct OggVorbis_File;
struct vorbis_info;

namespace BFG {
namespace Audio {

//! This class read and decode audio-data from an OGG vorbis files.
class AUDIO_BLASTER_API OggFile : public AudioFile
{

static const unsigned int AB_16BIT_SAMPLES = 2;
static const unsigned int AB_LITTLEENDIAN = 0;
static const unsigned int AB_SIGNED = 1;

public:
	OggFile(const std::string& filename);
	~OggFile();

	void read(ALuint bufferID);

private:

	unsigned long decode(char *buffer, unsigned long bufferSize);
	void close();
	void open(const std::string& filename);

	const unsigned int BUFFER_SIZE;
	OggVorbis_File* mVorbisFile;
	vorbis_info* mFileInfo;
	boost::scoped_array<char> mBuffer;
	ALenum mFormat;
};

} // namespace AudioBlaster
} // namespace BFG

#endif