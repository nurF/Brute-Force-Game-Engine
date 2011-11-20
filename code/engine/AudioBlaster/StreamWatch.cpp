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

#include <AudioBlaster/StreamWatch.h>

#include <boost/foreach.hpp>
#include <Base/Cpp.h>
#include <Base/CLogger.h>
#include <Core/ClockUtils.h>
#include <AudioBlaster/HelperFunctions.h>

namespace BFG {
namespace Audio {

StreamWatch::StreamWatch(const std::vector<std::string>& fileList): 
	mEventLoop(true, new EventSystem::BoostThread<>("StreamWatchWorkerThread."))
{
	init(fileList);
	mEventLoop.addEntryPoint(new Base::CEntryPoint(pseudoEntryPoint));
	mEventLoop.registerLoopEventListener(this, &StreamWatch::loopEventHandler);
	mEventLoop.run();
	mState = RUNNING;
};

StreamWatch::~StreamWatch() 
{
	mEventLoop.setExitFlag(true);
	mEventLoop.doLoop();

	// Let a little time for the event system to clean up it's stuff
	dbglog << "StreamWatch instance deleted.";
	boost::this_thread::sleep(boost::posix_time::milliseconds(200));

	mBusyStreams.clear();
	mReadyStreams.clear();
}

void StreamWatch::init(const std::vector<std::string>& filelist)
{
	BOOST_FOREACH(std::string str, filelist)
	{
		ReadyStreamsT::iterator it = mReadyStreams.find(str);

		if (it == mReadyStreams.end())
			createStream(str);
	}
}

void StreamWatch::createStream(const std::string& streamName)
{
	dbglog << "Lookup if ready stream already exists.";
	boost::mutex::scoped_lock lock(mMutex);

	ReadyStreamsT::iterator it = mReadyStreams.find(streamName);

	if (it != mReadyStreams.end())
	{
		dbglog << "Stream of that file already exists. Continue without creating new stream object.";
		return;
	}

	dbglog << "Creating stream of: "+streamName;
	mReadyStreams[streamName] = boost::shared_ptr<Stream>(new Stream(streamName));
	dbglog << "Stream created";
}

ALuint StreamWatch::demandStream(const std::string& streamName, 
								 boost::function<void (void)> onFinishCallback, 
								 ALuint aSourceId)
{
	ReadyStreamsT::iterator it = mReadyStreams.find(streamName);

	if (it == mReadyStreams.end())
		throw std::logic_error("Stream ID not known at StreamWatch::demandStream.");

	boost::shared_ptr<Stream> tempPtr = it->second;

	ALuint sourceId;

	if (!aSourceId)
	{	
		// Read error memory to get a valid result.
		ALenum error = alGetError();
		alGenSources(1, &sourceId);
		error = alGetError();	

		if (error != AL_NO_ERROR)
			throw std::logic_error("Creation of OpenAl source failed at StreamWatch::demandStream. AL_Error: "+stringifyAlError(error));
	}
	else
		sourceId = aSourceId;

	if(!alIsSource(sourceId))
		throw std::logic_error("Generated sourceID is no valid sourceID. Problem with OpenAL. Error at StreamWatch::demandStream.");

	tempPtr->startStream(sourceId, onFinishCallback);

	try
	{
		boost::mutex::scoped_lock lock(mMutex);

		dbglog << "Put stream to busy stream list.";
		mBusyStreams.push_back(tempPtr);

		dbglog << "Remove stream from ready stream list.";
		mReadyStreams.erase(it);
	}
	catch (std::exception e)
	{
		throw std::logic_error("Exception occurred while demanding a new stream.");
	}

	// Create a new stream with state 'ready'. If a stream of this file will requested again, a ready stream stands ready.
	createStream(streamName);

	return sourceId;
}

void StreamWatch::loopEventHandler(LoopEvent* loopEvent)
{
	boost::this_thread::sleep(boost::posix_time::millisec(2));

//	dbglog << "Enter loopEventHandler";

	try
	{
		boost::mutex::scoped_lock lock(mMutex);
		erase_if(mBusyStreams, boost::bind(&Stream::state, _1) == Stream::FINISHED);
	}
	catch (std::exception e)
	{
		errlog << "Exception occurred while deleting finished stream with erase_if(..)";
		throw std::logic_error("Exception occurred while deleting finished stream with erase_if(..)");
	}

	StreamsT::iterator it = mBusyStreams.begin();

	for(; it != mBusyStreams.end(); ++it)
	{
		//dbglog << "Next stream step.";
		boost::mutex::scoped_lock lock(mMutex);
		(*it)->nextStreamStep();
	}
}

void StreamWatch::idle()
{
	if (mState == RUNNING)
	{
		mEventLoop.unregisterLoopEventListener(this);
		mBusyStreams.clear();
		mState = IDLE;
	}
	else
		throw std::logic_error("Calling StreamWatch::idle() not allowed while state is not RUNNING.");
}

void StreamWatch::freeze()
{
	if (mState == RUNNING)
	{
			mEventLoop.unregisterLoopEventListener(this);
			mState = FREEZED;
	}
	else
		throw std::logic_error("Calling StreamWatch::freeze() not allowed while state is not RUNNING.");
}

void StreamWatch::reactivate()
{
	mEventLoop.registerLoopEventListener(this, &StreamWatch::loopEventHandler);
	mState = RUNNING;
}

} // namespace AudioBlaster
} // namespace BFG
