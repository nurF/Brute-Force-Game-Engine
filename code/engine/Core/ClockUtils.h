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

#ifndef BFG_SMART_CLOCK_
#define BFG_SMART_CLOCK_

#include <boost/thread/thread.hpp>
#include <boost/date_time.hpp>
#include <boost/date_time/posix_time/ptime.hpp>

#include <Core/Defs.h>

namespace BFG {
namespace Clock {

enum Resolution
{
	second = 1,
	milliSecond = 1000,
	microSecond = 1000000
};

namespace bpt = boost::posix_time;

//! This class provides basic boost::date_time features in a handy manner.
class BFG_CORE_API Base
{

public:
	virtual ~Base() {}

	//! \return The current time
	bpt::ptime now();
	
	//! \brief Calculates a duration from a given time to now
	bpt::time_duration tillNow(bpt::ptime from);
	
	//! \return A duration in a given resolution (s, ms, mics)
	long total(bpt::time_duration, Resolution resolution);
};

//! This class works like a stop watch, with one exception: stop() could be
//! called many times and it gives the duration from start till stop has
//! been called.
class BFG_CORE_API StopWatch : public Base
{

public:

	StopWatch(Resolution resolution);

	//! Like a stopwatch the time will be measured from the moment when start
	//! is pressed.
	void start();

	//! Returns the measured value in defined resolution.
	//! You can call stop() many times but not before calling start initial!
	long stop();

	//! Returns the measured time and restarts the clock
	long restart();
	
	//! Call this at the end of your clock operations to avoid wrong results
	//! by calling stop before start.
	void reset() { mRunning = false; };

private:

	Resolution  mResolution;
	bpt::ptime  mStartTime;

	//! Calling stop() before calling start() will result in a logic error.
	bool mRunning;
};


class BFG_CORE_API SleepFrequently
{
public:
	SleepFrequently(Resolution resolution,
	                long frequency);

	//! Measures the time between two ticks and sleeps the threat for the time
	//! that is left. It slows down the speed of a thread to a defined frequency.
	long measure();
	void reset();

private:

	//! Sleep this thread for a given duration in defined resolution.
	void  sleep(long duration);

	long  mFrequency;
	long  mTickCount;

	Resolution mResolution;

	StopWatch mOverall;
	StopWatch mOneCycle;

	bpt::ptime  mStartTime;
};

} // namespace Clock
} // namespace BFG

#endif
