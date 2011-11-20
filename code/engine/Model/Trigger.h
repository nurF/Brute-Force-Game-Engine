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

#ifndef TRIGGER_H__
#define TRIGGER_H__

#include <set>

#include <Base/CLogger.h>
#include <Core/Math.h>

#include <EventSystem/Emitter.h>

#include <Model/Events/TriggerEvent.h>
#include <Model/Managed.h>
#include <Model/Environment.h>

namespace BFG {

typedef std::vector<GameHandle> HandleVector;

namespace Trigger {

template <typename EventT>
struct DefaultFilter
{
	typedef std::set<GameHandle> ContainerT;
	
	static bool Applies(const ContainerT& filter, EventT* te)
	{
		assert(! "TODO: This is broken, adapt for new event here");
		//return filter.find(te->getData().mGOhandle) == filter.end();
		return true;
	}
};

template <typename FilterPolicy = DefaultFilter<Trigger::Event> >
class Sequence : public Managed,
                 boost::noncopyable,
                 Emitter
{
public:
	typedef typename FilterPolicy::ContainerT FilterContainerT;
	
	Sequence(EventLoop* loop,
	         GameHandle handle,
	         const HandleVector& goHandles,
	         const Environment& environment,
	         const FilterContainerT& filter,
	         f32 radius,
	         bool activated = true) :
	Managed(handle, "Trigger::Sequence", ID::OT_Trigger),
	Emitter(loop),
	mGoHandles(goHandles),
	mEnvironment(environment),
	mFilter(filter),
	mRadius(radius),
	mActivated(activated),
	mCurrentWP(0)
	{
		assert(! goHandles.empty() && "Trigger::Sequence has no handles!");
	
		for (int i=ID::TE_CHECK_LOCATION; i<ID::TE_ENDTRIGGEREVENTS; ++i)
		{
			loop->connect(i, this, &Sequence::EventHandler);
		}
	}
	                
	~Sequence()
	{
		for (int i=ID::TE_CHECK_LOCATION; i<ID::TE_ENDTRIGGEREVENTS; ++i)
		{
			loop()->disconnect(i, this);
		}
	}
	
	void EventHandler(Trigger::Event* te)
	{
		if (FilterPolicy::Applies(mFilter, te))
			return;
		
		switch(te->getId())
		{
		case ID::TE_CHECK_LOCATION:
			if (! mActivated)
				return;
			assert(! "TODO: This is broken, adapt for new event here");
//			onCheckLocation(te->getData().mGOhandle, boost::get<v3>(te->getData().mValue));
			break;

		case ID::TE_RESET:
			onReset();
			break;

		default:
			break;
		}
	}
	
private:
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame)
	{
	}

	void onCheckLocation(GameHandle handle, const v3& position)
	{
		// Note: This can't be done in internalUpdate as we would be bothered
		//       to store every position of every object as member variable.
		//       I thought about adding a "mLatest" member var, but this can't
		//       be done, too, because internalUpdate is called once per tick
		//       whereas EventHandler might be called X-times per tick.

		GameHandle currentHandle = mGoHandles.at(mCurrentWP);

		const BFG::Location& go = mEnvironment.getGoValue<BFG::Location>(currentHandle, ID::PV_Location, ValueId::ENGINE_PLUGIN_ID);
		
		if (nearEnough(go.position, position, mRadius))
		{
			Fire(ID::TE_FIRED, handle, currentHandle);
			++mCurrentWP;
			
			if (mCurrentWP >= static_cast<s32>(mGoHandles.size()))
			{
				Fire(ID::TE_SEQUENCE_COMPLETE, handle, currentHandle);
				mActivated = false;
			}
		}
	}
	
	void onReset()
	{
		mCurrentWP = 0;
		mActivated = true;
	}
	
	void Fire(ID::ModelAction triggerEvent, GameHandle handle, GameHandle wpOwner) const
	{
		assert(! "TODO: Emit a new event here!");
#if 0
		emit<Trigger::Event>
		(
			triggerEvent,
			handle,
			wpOwner,
			getHandle(),
			mCurrentWP
		);
#endif

		infolog << "Trigger fired ("
		        << ID::asStr(triggerEvent)
		        << ") by " << handle;
	}


	HandleVector                mGoHandles;
	const Environment&          mEnvironment;
	FilterContainerT            mFilter;
	f32                         mRadius;
	bool                        mActivated;
	s32                         mCurrentWP;
};

} // namespace Trigger
} // namespace BFG

#endif
