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

#ifndef AUDIO_BLASTER_OBJECT_H
#define AUDIO_BLASTER_OBJECT_H

#include <AudioBlaster/Defines.h>

#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <al.h>

#include <Core/Types.h>
#include <Core/Utils.h>
#include <Core/v3.h>

#include <AudioBlaster/AudioEvent.h>
#include <AudioBlaster/Transport.h>
#include <AudioBlaster/OpenAlUpdateHandler.h>

namespace BFG {
namespace Audio {

class StreamWatch;
class AudioBlaster;

class AUDIO_BLASTER_API AudioObject
{
public:
	AudioObject(GameHandle gameHandle, 
	            const std::string& audioName,
	            boost::shared_ptr<StreamWatch> streamWatch,
	            const AOPresetData& presetData,
	            const AOLocation& location,
				AudioBlaster& audioBlaster,
	            bool updateable = false);

	~AudioObject();

private:
	void initEvents();
	void initPresetData(const AOPresetData& presetData);
	void initLocation(const AOLocation& locationData);
	void eventHandler(AudioEvent* AE);
	void onStreamFinished();

	ALuint mSourceId;
	GameHandle mGameHandle;
	std::string mAudioName;
	bool mUpdateable;
	AudioBlaster& mAudioBlaster;

	boost::scoped_ptr<OpenAlUpdateHandler> mUpdateHandler;
	boost::shared_ptr<StreamWatch> mStreamWatch;
};

} // namespace AudioBlaster
} // namespace BFG

#endif
