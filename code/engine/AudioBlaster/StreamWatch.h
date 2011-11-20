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

#ifndef AB_STREAM_WATCH
#define AB_STREAM_WATCH

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/function.hpp>
#include <EventSystem/Core/EventLoop.h>
#include <AudioBlaster/Defines.h>
#include <AudioBlaster/Stream.h>

namespace BFG {
namespace Audio {

class AUDIO_BLASTER_API StreamWatch
{

public:

	StreamWatch(const std::vector<std::string>& fileList);
	~StreamWatch();

	//! An audio-object can demand a stream.
	ALuint demandStream(const std::string& streamName, 
						boost::function<void (void)>,
						ALuint aSourceID = 0);

	//! All files in ready list stay saved. Busy streams will be deleted and the working thread holds.
	void idle();
	//! T he worker thread holds but all ready streams will freeze at their current position.
	void freeze();
	//! reactivate the Instance from freeze or idle.
	void reactivate();

private:

	enum State
	{
		RUNNING,
		IDLE,
		FREEZED
	};

	boost::mutex mMutex; 

	//! Entry point is the constructor that calls run. So we need a dummy entry point to satisfy eventsystem policy.
	static void* pseudoEntryPoint(void * iPointer) { return 0; }
	void loopEventHandler(LoopEvent* loopEvent);

	//! Open one stream for every file that every file is preloaded if a stream will be demanded.
	void init(const std::vector<std::string>& filelist);

	void createStream(const std::string& streamName);

	//! A stream list that safes streams of one audio-file.
	typedef std::vector<boost::shared_ptr<Stream> > StreamsT;
	typedef std::map<std::string, boost::shared_ptr<Stream> > ReadyStreamsT;

	ReadyStreamsT mReadyStreams;
	StreamsT mBusyStreams;
	
	EventLoop mEventLoop;
	State mState;
};

} // namespace AudioBlaster
} // namespace BFG

#endif