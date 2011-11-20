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

#ifndef AUDIO_BLASTER_STREAMER_H
#define AUDIO_BLASTER_STREAMER_H

#include <AudioBlaster/Defines.h>
#include <boost/shared_ptr.hpp>
#include <boost/scoped_array.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <al.h>

namespace BFG {
namespace Audio {

class AudioFile;

//! This class provides streaming audio data in OpenAl buffers.
//! An openAl buffers are represented as BufferIds (ALuint == unsigned int).
//! So we take only care about these IDs OpenAl take care about the rest.
class AUDIO_BLASTER_API Stream
{

friend class StreamWatch;

enum State
{
	READY = 1,
	STREAMING,
	INITIALIZING,
	FINISHED
};

public:
	Stream(const std::string& name);
	~Stream();

	//! Open a new thread for the stream process.
	void startStream(ALuint sourceId, boost::function<void (void)> onStreamFinished);

	void nextStreamStep();

	State state() { return mState; }

private:

	//! Initialize the buffers with first audio data before the stream process is started.
	void preload();

	const int mNUM_BUFFER;
	const std::string mName;

	ALuint mSourceId;
	State mState;
	boost::function<void (void)> mOnStreamFinished;

	boost::scoped_array<ALuint> mBufferIds;
	boost::shared_ptr<AudioFile> mAudioFile;
};

} // namespace AudioBlaster
} // namespace BFG

#endif