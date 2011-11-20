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


#ifndef AUDIO_BLASTER_TRANSPORT_H
#define AUDIO_BLASTER_TRANSPORT_H

#include <Core/Types.h>
#include <Core/v3.h>
#include <Core/Utils.h>
#include <AudioBlaster/Enums.hh>

namespace BFG {

struct AOLocation
{
	AOLocation(const v3& position = v3(0,0,0),
	           const v3& orientation = v3(0,0,0),
			   const v3& velocity = v3(0,0,0)):
		mPosition(position),
		mOrientation(orientation),
		mVelocity(velocity)
	{}

	v3 mPosition;
	v3 mOrientation;
	v3 mVelocity;
};

struct AOPresetData
{
	AOPresetData(float gain,
	             float pitch,
	             float coneInnerAngle,
	             float coneOuterAngle,
	             float coneOuterGain,
	             float referenceDistance,
	             float rolloffFactor):
	    mGain(gain),
		mPitch(pitch),
		mConeInnerAngle(coneInnerAngle),
		mConeOuterAngle(coneOuterAngle),
		mConeOuterGain(coneOuterGain),
		mReferenceDistance(referenceDistance),
		mRolloffFactor(rolloffFactor)
	{}

	float mGain;
	float mPitch;
	float mConeInnerAngle;
	float mConeOuterAngle;
	float mConeOuterGain;
	float mReferenceDistance;
	float mRolloffFactor;
};

struct AOCreation
{

AOCreation(GameHandle handle,
		   const std::string& name,
		   ID::AudioObjectType aot = ID::AOT_SINGLE_SERVING,
		   ID::AudioPreset aop = ID::AOP_DEFAULT_SOUND,
		   const AOLocation& location = AOLocation()):
mHandle(handle),
mName(name),
mAudioObjectType(aot),
mAudioPreset(aop),
mLocation(location)
{}

GameHandle  mHandle;
std::string mName;
ID::AudioObjectType mAudioObjectType;
ID::AudioPreset mAudioPreset;
AOLocation  mLocation;
};

} // namespace BFG
#endif