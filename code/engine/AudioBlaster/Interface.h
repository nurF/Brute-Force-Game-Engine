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

#ifndef BFG_AUDIO_INTERFACE_H
#define BFG_AUDIO_INTERFACE_H

#include <boost/scoped_ptr.hpp>
#include <Base/CEntryPoint.h>
#include <AudioBlaster/Defines.h>

namespace BFG {
namespace Audio {

class AudioMain;

class AUDIO_BLASTER_API AudioInterface
{
public:
	friend class Base::CClassEntryPoint<AudioInterface> ;

	// This is your hooking place
	static Base::IEntryPoint* getEntryPoint();

private:
	void* startAudio(void* ptr);
	
	boost::scoped_ptr<AudioMain> mAudioMain;
};

} // namespace AudioBlaster
} // namespace BFG

#endif

