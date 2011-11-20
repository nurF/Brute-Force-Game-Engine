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

#include <AudioBlaster/OggFile.h>

#include <stdexcept>
#include <stdlib.h>
#include <vorbis/vorbisfile.h>
#include <Base/CLogger.h>

namespace BFG {
namespace Audio {

OggFile::OggFile(const std::string& filename) : BUFFER_SIZE(44100)
{
	mBuffer.reset(new char[BUFFER_SIZE]);
	open(filename);
}

OggFile::~OggFile()
{
	close();
}

void OggFile::open(const std::string& filename)
{
	mVorbisFile = new OggVorbis_File;

	if (ov_fopen(filename.c_str(), mVorbisFile))
		throw std::logic_error(std::string("AudioBlaster: Error loading file: "+ filename +" ."));

	mFileInfo = ov_info(mVorbisFile, -1);

	if (mFileInfo->channels == 1)
		mFormat = AL_FORMAT_MONO16;
	else if (mFileInfo->channels == 2)
		mFormat = AL_FORMAT_STEREO16;
}

void OggFile::close()
{
	ov_clear(mVorbisFile);
}

void OggFile::read(ALuint bufferID)
{
	unsigned long bytesDecoded = decode(mBuffer.get(), BUFFER_SIZE);
	alBufferData(bufferID, mFormat, mBuffer.get(), bytesDecoded, mFileInfo->rate);
}

unsigned long OggFile::decode(char *buffer, unsigned long bufferSize)
{
	int currentSection;
	long decodeSize;
	unsigned long bytesDone = 0;

	do
	{
		decodeSize = ov_read
		(
			mVorbisFile, 
			buffer + bytesDone, 
			bufferSize - bytesDone, 
			AB_LITTLEENDIAN,
			AB_16BIT_SAMPLES, 
			AB_SIGNED,
			&currentSection
		);

		bytesDone += decodeSize;
	}
	while (decodeSize > 0 && bytesDone < bufferSize);

	return bytesDone;
}

} // namespace AudioBlaster
} // namespace BFG