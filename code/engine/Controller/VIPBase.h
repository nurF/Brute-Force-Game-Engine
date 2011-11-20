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

#ifndef _VIPBASE_H_
#define _VIPBASE_H_

#include <set>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>

#include <EventSystem/Emitter.h>

#include <Controller/ControllerEvents_fwd.h>
#include <Controller/Enums.hh>
#include <Controller/Axis.h>
#include <Controller/Button.h>

// Used by Controller.cpp and VIPComponents.h
#define ONE_SEC_IN_MICROSECS 1000000

namespace BFG {
namespace Controller_ {

class State;

namespace VIP {

class CommonBase : Emitter
{
public:
	CommonBase(EventIdT aAid,
	           State* aState,
	           std::set<ButtonCodeT> aRelevantButtons,
	           std::set<AxisNumeratorT> aRelevantAxis,
	           ID::DeviceType aDevice);
	
	virtual ~CommonBase()
	{
	}
	
	//! \note There's no way around. A VIP must be able to receive
	//!       all types of data in order to work correctly.
	//!       I tried hard to put these two functions under a hat,
	//!       but I failed.
	virtual void FeedAxisData(ID::DeviceType,
	                          ID::AxisType,
	                          const AxisData<int>&)
	{
	}

	//! \see FeedAxisData()
	virtual void FeedButtonData(ID::DeviceType,
	                            ID::ButtonState,
	                            ButtonCodeT)
	{
	}

	EventIdT getAction() const
	{
		return mAid;
	}
	
	virtual VipPayloadT getResult() const = 0;
	
	//! Optional. Only used if feedback is needed.
	//! This gets always called by State. (r636)
	virtual void FeedTime(long /*microseconds_passed*/)
	{
	}
	
	bool NeedsTime() const
	{
		return mUsingFeedback;
	}

	void EnableFeedback()
	{
		mUsingFeedback = true;
	}
	
	void DisableFeedback()
	{
		mUsingFeedback = false;		
	}

	void Emit();

public:
	std::set<ButtonCodeT>    mRelevantButtons;
	std::set<AxisNumeratorT> mRelevantAxis;
	ID::DeviceType           mDevice;

protected:
	EventIdT mAid;
	State*   mState;

private:
	bool     mUsingFeedback;
};

} // namespace VIP

typedef boost::shared_ptr<VIP::CommonBase> VipPtrT;

namespace VIP {

template <typename DeviceT>
void assignToDevice(VipPtrT vip, DeviceT& device)
{
	if (! device.good())
		return;

	BOOST_FOREACH(AxisNumeratorT axis, vip->mRelevantAxis)
	{
		device.registerVIP(ID::AT_Normal, axis, vip);
	}
	
	BOOST_FOREACH(ButtonCodeT button, vip->mRelevantButtons)
	{
		device.registerVIP(button, vip);
	}
}

} // namespace VIP

} // namespace Controller_
} // namespace BFG

#endif
