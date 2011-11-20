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


#ifndef AUDIO_BLASTER_OPENAL_UPDATE_HANDLER_H
#define AUDIO_BLASTER_OPENAL_UPDATE_HANDLER_H

#include <AudioBlaster/Defines.h>
#include <boost/shared_ptr.hpp>
#include <al.h>
#include <Core/Types.h>
#include <Core/v3.h>
#include <AudioBlaster/AudioEvent.h>

namespace BFG {
namespace Audio {

class StreamWatch;

class AUDIO_BLASTER_API OpenAlUpdateHandler
{
public:
	OpenAlUpdateHandler(ALuint sourceID, GameHandle handle);
	~OpenAlUpdateHandler();

private:
	void initEvents();
	void eventHandler(AudioEvent* AE);

	void onEventPosition(const AudioPayloadT& payload);
	void onEventOrientation(const AudioPayloadT& payload);
	void onEventVelocity(const AudioPayloadT& payload);

	void onEventPitch(const AudioPayloadT& payload);
	void onEventGain(const AudioPayloadT& payload);
	void onEventRolloffFactor(const AudioPayloadT& payload);
	void onEventReferenceDistance(const AudioPayloadT& payload);
	void onEventConeOuterGain(const AudioPayloadT& payload);
	void onEventConeInnerAngle(const AudioPayloadT& payload);
	void onEventConeOuterAngle(const AudioPayloadT& payload);

	ALuint mSourceId;
	GameHandle mHandle;
};

} // namespace AudioBlaster
} // namespace BFG

#endif
