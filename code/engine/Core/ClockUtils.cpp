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

#include <Core/ClockUtils.h>

namespace BFG {
namespace Clock {

using namespace boost::posix_time;
	
StopWatch::StopWatch(Resolution resolution) :
mResolution(resolution),
mRunning(false)
{
}

ptime Base::now()
{
	return microsec_clock::local_time();
}

time_duration Base::tillNow(ptime from)
{
	return time_period(from, now()).length();
}

long Base::total(time_duration duration, Resolution resolution)
{
	switch (resolution)
	{
		case microSecond:
			return static_cast<long>(duration.total_microseconds());
		case milliSecond:
			return static_cast<long>(duration.total_milliseconds());
		case second:
			return static_cast<long>(duration.total_seconds());
		default:
			throw std::logic_error
				("Clock::Base::Total: Unknown time resolution used");
	}
}

void StopWatch::start()
{
	mStartTime = now();
	mRunning = true;
}

long StopWatch::stop()
{
	if (!mRunning)
		throw std::logic_error("Don't call stop before start has been called!");

	return total(tillNow(mStartTime), mResolution);
}

long StopWatch::restart()
{
	long timeSinceStart = stop();
	start();
	return timeSinceStart;
}

SleepFrequently::SleepFrequently(Resolution resolution,
                                 long frequency) :
mFrequency(frequency),
mResolution(resolution),
mOverall(resolution),
mOneCycle(resolution)
{
	reset();
}

void SleepFrequently::reset()
{
	mStartTime = microsec_clock::local_time();
	mTickCount = 0;
	mOverall.start();
	mOneCycle.start();
}

long SleepFrequently::measure()
{
	++mTickCount;

	long totalTimePassed = mOverall.stop();
	long timeOffset = (mFrequency * mTickCount) - totalTimePassed;

	if (timeOffset > mFrequency)
	{
		sleep(timeOffset);
	}

	long diffTimePassed = mOneCycle.stop();
	mOneCycle.start();

	return diffTimePassed;
}

void SleepFrequently::sleep(long offset)
{
	//std::cout << "sleep:" << offset << "\n";
	switch (mResolution)
	{
		case microSecond:
			boost::this_thread::sleep(microseconds(offset));
		break;
		case milliSecond:
			boost::this_thread::sleep(milliseconds(offset));
		break;
		case second:
			boost::this_thread::sleep(seconds(offset));
		break;
	}
}

} // namespace Clock
} // namespace BFG
