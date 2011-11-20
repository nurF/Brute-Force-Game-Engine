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

#include <AudioBlaster/AudioObject.h>
#include <EventSystem/Core/EventLoop.h>

#include <AudioBlaster/AudioBlaster.h>
#include <AudioBlaster/Enums.hh>
#include <AudioBlaster/Main.h>
#include <AudioBlaster/FileFactory.h>
#include <AudioBlaster/StreamWatch.h>

namespace BFG {
namespace Audio {

AudioObject::AudioObject(GameHandle gameHandle, 
                         const std::string& audioName,
						 boost::shared_ptr<StreamWatch> streamWatch,
						 const AOPresetData& presetData,
						 const AOLocation& location,
						 AudioBlaster& audioBlaster,
						 bool updateable) : 
mGameHandle(gameHandle),
mAudioName(audioName),
mUpdateHandler(NULL),
mStreamWatch(streamWatch),
mSourceId(0),
mAudioBlaster(audioBlaster),
mUpdateable(updateable)
{
	initEvents();
	initLocation(location);
	initPresetData(presetData);

	if (updateable)
		mUpdateHandler.reset(new OpenAlUpdateHandler(mSourceId, mGameHandle));
}

void AudioObject::initEvents()
{
	AudioMain::eventLoop()->connect(ID::AE_PLAY, this, &AudioObject::eventHandler, mGameHandle);
	AudioMain::eventLoop()->connect(ID::AE_STOP, this, &AudioObject::eventHandler, mGameHandle);
	AudioMain::eventLoop()->connect(ID::AE_PAUSE, this, &AudioObject::eventHandler, mGameHandle);
}

void AudioObject::initPresetData(const AOPresetData& pd)
{
	alSourcef(mSourceId, AL_GAIN, pd.mGain);
	alSourcef(mSourceId, AL_PITCH, pd.mPitch);
	alSourcef(mSourceId, AL_CONE_INNER_ANGLE, pd.mConeInnerAngle);
	alSourcef(mSourceId, AL_CONE_OUTER_ANGLE, pd.mConeOuterAngle);
	alSourcef(mSourceId, AL_CONE_OUTER_GAIN, pd.mConeOuterGain);
	alSourcef(mSourceId, AL_REFERENCE_DISTANCE, pd.mReferenceDistance);
	alSourcef(mSourceId, AL_ROLLOFF_FACTOR, pd.mRolloffFactor);
}

void AudioObject::initLocation(const AOLocation& lc)
{
	alSource3f(mSourceId, AL_POSITION, lc.mPosition.x, lc.mPosition.y, lc.mPosition.z);
	alSource3f(mSourceId, AL_ORIENTATION, lc.mOrientation.x, lc.mOrientation.y, lc.mOrientation.z);
	alSource3f(mSourceId, AL_VELOCITY, lc.mVelocity.x, lc.mVelocity.y, lc.mVelocity.z);
}

AudioObject::~AudioObject()
{
	AudioMain::eventLoop()->disconnect(ID::AE_PLAY, this);
	AudioMain::eventLoop()->disconnect(ID::AE_STOP, this);
	AudioMain::eventLoop()->disconnect(ID::AE_PAUSE, this);
}

// boost::function<int (int)> f;
// X x;
// f = std::bind1st(
//				 std::mem_fun(&X::foo), &x);
// f(5); // Call x.foo(5)


void AudioObject::eventHandler(AudioEvent* AE)
{
	switch (AE->getId())
	{
		case ID::AE_PLAY:
		{
			
			ALint result;
			alGetSourcei(mSourceId, AL_SOURCE_STATE, &result);

			if (result != AL_PAUSED)
			{
				if (mSourceId != 0)
					alDeleteSources(1, &mSourceId);
	
					boost::function<void(void)> onFinishedCallback = boost::bind
					(
						&AudioObject::onStreamFinished,
						this
					);
					
					mSourceId = mStreamWatch->demandStream
					(
						mAudioName,
						onFinishedCallback
					);
			}

			alSourcePlay(mSourceId);

			break;
		}
		case ID::AE_STOP:
			alSourceStop(mSourceId);
			break;
		case ID::AE_PAUSE:
			alSourcePause(mSourceId);
			break;

		default:
			throw std::logic_error("Unhandled event!");
	}
}

void AudioObject::onStreamFinished()
{
	if (!mUpdateable)
	{
		// release openAl ressource.
		alDeleteSources(1, &mSourceId);
		mSourceId = 0;
		mAudioBlaster.deleteAudioObject(mGameHandle);
	}
}

} // namespace AudioBlaster
} // namespace BFG