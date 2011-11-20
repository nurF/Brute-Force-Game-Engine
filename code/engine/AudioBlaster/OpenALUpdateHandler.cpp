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

#include <AudioBlaster/OpenAlUpdateHandler.h>
#include <EventSystem/Core/EventLoop.h>
#include <AudioBlaster/Enums.hh>
#include <AudioBlaster/Main.h>

namespace BFG {
namespace Audio {

OpenAlUpdateHandler::OpenAlUpdateHandler(ALuint sourceID, GameHandle handle):
mSourceId(sourceID),
mHandle(handle)
{
	initEvents();
}

void OpenAlUpdateHandler::initEvents()
{
	AudioMain::eventLoop()->connect(ID::AE_PITCH, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_GAIN, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_ROLLOFF_FACTOR, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_REFERENCE_DISTANCE, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_CONE_OUTER_GAIN, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_CONE_INNER_ANGLE, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_CONE_OUTER_ANGLE, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_POSITION, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_ORIENTATION, this, &OpenAlUpdateHandler::eventHandler, mHandle);
	AudioMain::eventLoop()->connect(ID::AE_VELOCITY, this, &OpenAlUpdateHandler::eventHandler, mHandle);
}

OpenAlUpdateHandler::~OpenAlUpdateHandler()
{
	AudioMain::eventLoop()->disconnect(ID::AE_PITCH, this);
	AudioMain::eventLoop()->disconnect(ID::AE_GAIN, this);
	AudioMain::eventLoop()->disconnect(ID::AE_ROLLOFF_FACTOR, this);
	AudioMain::eventLoop()->disconnect(ID::AE_REFERENCE_DISTANCE, this);
	AudioMain::eventLoop()->disconnect(ID::AE_CONE_OUTER_GAIN, this);
	AudioMain::eventLoop()->disconnect(ID::AE_CONE_INNER_ANGLE, this);
	AudioMain::eventLoop()->disconnect(ID::AE_CONE_OUTER_ANGLE, this);
	AudioMain::eventLoop()->disconnect(ID::AE_POSITION, this);
	AudioMain::eventLoop()->disconnect(ID::AE_ORIENTATION, this);
	AudioMain::eventLoop()->disconnect(ID::AE_VELOCITY, this);
}

void OpenAlUpdateHandler::eventHandler(AudioEvent* AE)
{
	switch (AE->getId())
	{
		case ID::AE_PITCH:
			onEventPitch(AE->getData());
			break;
		case ID::AE_GAIN:
			onEventGain(AE->getData());
			break;
		case ID::AE_ROLLOFF_FACTOR:
			onEventRolloffFactor(AE->getData());
			break;
		case ID::AE_REFERENCE_DISTANCE:
			onEventReferenceDistance(AE->getData());
			break;
		case ID::AE_CONE_OUTER_GAIN:
			onEventConeOuterGain(AE->getData());
			break;
		case ID::AE_CONE_INNER_ANGLE:
			onEventConeInnerAngle(AE->getData());
			break;
		case ID::AE_CONE_OUTER_ANGLE:
			onEventConeOuterAngle(AE->getData());
			break;
		case ID::AE_POSITION:
			onEventPosition(AE->getData());
			break;
		case ID::AE_ORIENTATION:
			onEventOrientation(AE->getData());
			break;
		case ID::AE_VELOCITY:
			onEventVelocity(AE->getData());
			break;
		default:
			throw std::logic_error("Unhandled event!");
	}
}

void OpenAlUpdateHandler::onEventPitch(const AudioPayloadT& payload)
{
	ALfloat pitch = static_cast<ALfloat>(boost::get<float>(payload));
	alSourcef(mSourceId, AL_PITCH, pitch);
}

void OpenAlUpdateHandler::onEventGain(const AudioPayloadT& payload)
{
	ALfloat gain = static_cast<ALfloat>(boost::get<float>(payload));
	alSourcef(mSourceId, AL_GAIN, gain);
}

void OpenAlUpdateHandler::onEventRolloffFactor(const AudioPayloadT& payload)
{
	ALfloat rollOffFactor = static_cast<ALfloat>(boost::get<float>(payload));
	alSourcef(mSourceId, AL_ROLLOFF_FACTOR, rollOffFactor);
}

void OpenAlUpdateHandler::onEventReferenceDistance(const AudioPayloadT& payload)
{
	ALfloat referenceDistance = static_cast<ALfloat>(boost::get<float>(payload));
	alSourcef(mSourceId, AL_REFERENCE_DISTANCE, referenceDistance);	
}

void OpenAlUpdateHandler::onEventConeOuterGain(const AudioPayloadT& payload)
{
	ALfloat coneOuterGain = static_cast<ALfloat>(boost::get<float>(payload));
	alSourcef(mSourceId, AL_CONE_OUTER_GAIN, coneOuterGain);	
}

void OpenAlUpdateHandler::onEventConeInnerAngle(const AudioPayloadT& payload)
{
	ALfloat coneInnerAngle = static_cast<ALfloat>(boost::get<float>(payload));
	alSourcef(mSourceId, AL_CONE_INNER_ANGLE, coneInnerAngle);
}

void OpenAlUpdateHandler::onEventConeOuterAngle(const AudioPayloadT& payload)
{
	ALfloat coneOuterAngle = static_cast<ALfloat>(boost::get<float>(payload));
	alSourcef(mSourceId, AL_CONE_OUTER_ANGLE, coneOuterAngle);
}

void OpenAlUpdateHandler::onEventPosition(const AudioPayloadT& payload)
{
	v3 position = boost::get<v3>(payload);
	alSource3f(mSourceId, AL_POSITION, position.x, position.y, position.z);
}

void OpenAlUpdateHandler::onEventOrientation(const AudioPayloadT& payload)
{
	v3 orientation = boost::get<v3>(payload);
	alSource3f(mSourceId, AL_ORIENTATION, orientation.x, orientation.y, orientation.z);
}

void OpenAlUpdateHandler::onEventVelocity(const AudioPayloadT& payload)
{
	v3 velocity = boost::get<v3>(payload);
	alSource3f(mSourceId, AL_VELOCITY, velocity.x, velocity.y, velocity.z);
}

} // namespace AudioBlaster
} // namespace BFG