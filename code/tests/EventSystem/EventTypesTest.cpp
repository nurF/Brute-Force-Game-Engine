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

#ifdef _DEBUG
	#define _CRTDBG_MAP_ALLOC
#endif

#include <cstdlib>
#include <cstdio>
#include <iostream>

#include <EventSystem/Core/EventDefs.h>
#include <EventSystem/Core/EventLoop.h>


static void PrintHeader()
{
	std::cout << "\n\
-----------------------------------------------------------------------------\n\
This Executable file is part of the UltimateSpaceProject\n\
\n\
Copyright (c) 2008 UltimateSpaceProject, all rights reserved\n\
Visit: ultimatespaceproject.de for more details\n\
\n\
Actually this is closed source ! (may change in the future)\n\
\n\
There is no permission to any person obtaining a copy of this software and \n\
associated documentation files (the UltimateSpaceProject-\"Software\"), \n\
to deal with the Software, including the rights to copy, modify, merge,\n\
publish, distribute, sublicense, and/or sell copies of the Software.\n\
\n\
The above copyright notice and this permission applies to all derived products.\n\
For the usage of this Software, be aware of the following disclaimer :\n\
\n\
THE SOFTWARE IS PROVIDED \"AS IS\", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR\n\
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,\n\
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE\n\
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER\n\
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,\n\
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN\n\
THE SOFTWARE.\n\
-----------------------------------------------------------------------------\n\
\n\
Name:        Event-System\n\
Author:      Falk Tristram\n\
Created:     Dec 2008 / Jan 2009\n\
Description: Event-System Test Suite for Console\n\
\n\
---------------------------------------------------------------------------\n";
}

#include <boost/date_time/posix_time/posix_time.hpp>

#include <Base/TestAppDeco.h>

const size_t count = 5000000;
const size_t scalingRuns = 1000;
const size_t size =  sizeof(BaseEvent);

//EXPORT_EVENT(TBaseEvent<int>);
void testNativeTypes()
{
    //BaseEvent iEvent;
    TBaseEvent<int> iEvent(0,0);
    const int& lol = iEvent;
    int rofl = iEvent;
}

typedef int edType;
typedef TBaseEvent<edType> myEvent;

typedef int simpleType;
typedef TBaseEvent<simpleType> simpleEvent;


int main( int /*argc*/, const char* /*argv[]*/ )
{
    BFG::Base::Deco::h3("", 80);		
	BFG::Base::Deco::h3("Event System Type Test Suite", 80);		

	//	EventManager::Init();
	std::cout << "Some general Information\n"
			  << "sizeof(BaseEvent) " << sizeof(BaseEvent) << "\n"
			  << "sizeof(BaseEventPool)" << sizeof(BaseEventPool) << "\n";
	
	{
		boost::posix_time::ptime StartTime(boost::posix_time::microsec_clock::local_time());
		
		typedef TBaseEvent<int> myEvent;
		myEvent** iStore = new myEvent* [count];

		for ( size_t i = 0; i < count; ++i )
		{
			iStore[i] = new myEvent(0);
		}

		for ( size_t i = 0; i < count; ++i )
		{
			delete iStore[i];
		}

		delete [] iStore;

		boost::posix_time::ptime EndTime(boost::posix_time::microsec_clock::local_time());
		long total_time = (EndTime-StartTime).total_microseconds();

		std::cout << "Time Needed for classic new/delete "
			<< total_time/1000.0 << "ms" << std::endl;

	}

	{
		boost::posix_time::ptime StartTime(boost::posix_time::microsec_clock::local_time());
		
		EventPool<BaseEvent> * iPool = new EventPool<BaseEvent>();

		for ( size_t runs = 0; runs < scalingRuns; ++runs )
		{
			for ( size_t i = 0; i < count; ++i )
			{
				iPool->reserveNextStorage<BaseEvent>();
			}

			iPool->clear();
		}
		delete iPool;

		boost::posix_time::ptime EndTime(boost::posix_time::microsec_clock::local_time());
		long total_time = (EndTime-StartTime).total_microseconds();

		std::cout << "Time Needed for pool new/delete "
			<< total_time/ ( 1000.0 * scalingRuns ) << "ms" << std::endl;

	}

	{

		BaseEventPool * iPool = new BaseEventPool();

		for ( size_t i = 0; i < count; ++i )
		{
			iPool->reserveNextStorage<BaseEvent>();
		}
		boost::posix_time::ptime StartTime(boost::posix_time::microsec_clock::local_time());

		for ( size_t runs = 0; runs < scalingRuns; ++runs )
		{
			for ( size_t j = 0; j < 10; ++j )
			{
				BaseEventPool * cPool;

				cPool = (BaseEventPool*)iPool->clone();
				cPool->clear();
				delete cPool;
			}
		}
		iPool->clear();
		delete iPool;

		boost::posix_time::ptime EndTime(boost::posix_time::microsec_clock::local_time());
		long total_time = (EndTime-StartTime).total_microseconds();

		std::cout << "Time Needed for copying a full pool ten times "
			<< total_time/ ( 1000.0 * scalingRuns ) << "ms" << std::endl;

	}
#ifdef USE_SSE2
	{
		


		BaseEventPool * iPool = new BaseEventPool();

		for ( size_t i = 0; i < count; ++i )
		{
			iPool->reserveNextStorage<BaseEvent>();
		}
		boost::posix_time::ptime StartTime(boost::posix_time::microsec_clock::local_time());

		for ( size_t runs = 0; runs < scalingRuns; ++runs )
		{
			for ( size_t j = 0; j < 10; ++j )
			{
				BaseEventPool * cPool;

				cPool = (BaseEventPool*)iPool->CloneOptimized();
				cPool->clear();
				delete cPool;
			}
		}
		iPool->clear();
		delete iPool;

		boost::posix_time::ptime EndTime(boost::posix_time::microsec_clock::local_time());
		long total_time = (EndTime-StartTime).total_microseconds();

		std::cout << "Time Needed for SSE2-copying a full pool ten times "
			<< total_time/ ( 1000.0 * scalingRuns ) << "ms" << std::endl;

	}
#endif // USE_SSE2
  
	std::cout << "Press [return] to exit!" << std::endl;
	std::cin.clear();
	std::cin.ignore(std::cin.rdbuf()->in_avail()); 
	std::cin.get();
	
	return 0; 
}
