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
#include <AudioBlaster/Stream.h>

#include <Base/CLogger.h>

#include <AudioBlaster/FileBase.h>
#include <AudioBlaster/FileFactory.h>
#include <AudioBlaster/HelperFunctions.h>

namespace BFG {
namespace Audio {

Stream::Stream(const std::string& name):
mName(name), 
mSourceId(0),
mNUM_BUFFER(6),
mState(INITIALIZING)
{
	dbglog << "Stream constructor called";

	mAudioFile = createFile(name);
	mBufferIds.reset(new ALuint[mNUM_BUFFER]);

	// Generate OpenAl buffers.
	alGenBuffers(mNUM_BUFFER, mBufferIds.get());

	std::string result = stringifyAlError(alGetError());

	for(int i = 0; i < mNUM_BUFFER; ++i)
		if(!alIsBuffer(mBufferIds[i]))
		{
			std::string result = stringifyAlError(alGetError());
			throw std::logic_error("Creating OpenAL buffer failed at Stream constructor. ALerror: "+result);
		}

	dbglog << "Buffers created.";

	preload();

	mState = READY;
}

Stream::~Stream()
{
	alDeleteBuffers(mNUM_BUFFER, mBufferIds.get());
	dbglog << "Destructor: Stream: "+mName+" deleted.";
}

void Stream::preload()
{
 	for (int i = 0; i < mNUM_BUFFER; ++i)
	{
		mAudioFile->read(mBufferIds[i]);
	}

	dbglog << "Stream: "+mName+" preloaded";
}

void Stream::startStream(ALuint sourceId, boost::function<void (void)> onStreamFinished)
{
	dbglog << "Start stream: "+mName;

	mOnStreamFinished = onStreamFinished;
	mState = STREAMING;
	mSourceId = sourceId;

	ALenum error = alGetError();
	
	ALboolean res =	alIsSource(mSourceId);
	res = alIsBuffer(mBufferIds[0]);

	alSourceQueueBuffers(mSourceId, mNUM_BUFFER, mBufferIds.get());
	error = alGetError();
	std::string err = stringifyAlError(error);

	dbglog << "Stream started.";
}

void Stream::nextStreamStep()
{
	ALint processedBuffers = 0;
	ALuint tempBufferId;

	// Get the numbers of buffers that are already processed.
	alGetSourcei(mSourceId, AL_BUFFERS_PROCESSED, &processedBuffers);

	if (processedBuffers >= mNUM_BUFFER)
	{
		dbglog << "Stream '"+mName+"' finished. SetState to 'finished'";
		alSourceUnqueueBuffers(mSourceId, mNUM_BUFFER, mBufferIds.get());
		mState = FINISHED;

		mOnStreamFinished();

		return;
	}

	while (processedBuffers > 0)
	{
		alSourceUnqueueBuffers(mSourceId, 1, &tempBufferId);
		mAudioFile->read(tempBufferId);
		alSourceQueueBuffers(mSourceId, 1, &tempBufferId);
		alGetSourcei(mSourceId, AL_BUFFERS_PROCESSED, &processedBuffers);
	}
}

} // namespace AudioBlaster
} // namespace BFG
