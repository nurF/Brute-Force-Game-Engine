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

#ifndef _VIPCOMPONENTS_H_
#define _VIPCOMPONENTS_H_

#include <cassert>

#include <Core/Math.h>

#include <Base/CLogger.h>

#include <Controller/VIPBase.h>
#include <Controller/VIPEnvironment.h>

#ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable:4100) // "unreferenced local variable"
#endif

/**
	\file
	\verbatim

	NOTE
	====
	This file is still highly experimental and most components here, aren't
	"perfect" yet. They simply work, but that's all. So, refactoring them at
	the moment makes no sense, because future requirements are unclear and
	useful concepts cannot be worked out.

	However, it shouldn't be difficult to refactor them when the time has
	come. Read more in VIPGenerator.h.
	
	\endverbatim
*/

namespace BFG {
namespace Controller_ {
namespace VIP {

template <int VipID, typename Parent>
class Vip : public Parent
{
public:
	enum { ID = VipID };
	
	explicit Vip(typename Parent::EnvT& env) :
		Parent(env)
	{
	}
};

//! \todo Find a better name for this module.
template <typename Parent>
class FeedbackTrigger : public Parent
{
public:
	explicit FeedbackTrigger(typename Parent::EnvT& env) :
		Parent(env),
		mFrequency(0),
		mTimeDeficit(0)
	{
		if (env.mFrequency > 0)
			mFrequency = ONE_SEC_IN_MICROSECS / env.mFrequency;
	}

	virtual void onFeedback(long microseconds_passed) = 0;

protected:
	long mFrequency;
	long mTimeDeficit;

private:
	//! Overwrites CommonBase::FeedTime()
	virtual void FeedTime(long microseconds_passed)
	{
		if (mFrequency == 0)
			return;
	
		mTimeDeficit += microseconds_passed;

		// Be very careful here.
		// Each tiny change may introduce a new race condition.
		while (Parent::NeedsTime() && mTimeDeficit >= mFrequency)
		{
			mTimeDeficit -= mFrequency;
			onFeedback(microseconds_passed);
		}
		
		if (mTimeDeficit > mFrequency)
		{
			mTimeDeficit = mFrequency;
		}
	}
};

//! \todo Find a better name for this module.
template <typename Parent>
class Repeater : public Parent
{
public:
	explicit Repeater(typename Parent::EnvT& env) :
		Parent(env)
	{
	}
	
	virtual void onFeedback(long /*microseconds_passed*/)
	{
		this->Emit();
	}
};

#ifdef CONTROLLER_DEBUG

namespace {

void ButtonDataDebug(ID::DeviceType dt, ID::ButtonState bs, ButtonCodeT code)	
{
	std::string code_str = "[unknown device in use]";

	if (dt == ID::DT_Mouse)
	{
		code_str = ID::asStr(OIS::MouseButtonID(code));
	}
	else if (dt == ID::DT_Keyboard)
	{
		code_str = ID::asStr(ID::KeyboardButton(code));
	}
	else if (dt == ID::DT_Joystick)
	{
		code_str = ID::asStr(ID::JoystickButtonID(code));
	}
		
	dbglog << "BUTTONPROCESSOR GOT: " << ID::asStr(dt)
	       << " WITH STATE: " << ID::asStr(bs)
	       << " AND CODE: " << code_str;
}

void AxisDataDebug(ID::DeviceType dt, ID::AxisType at, const AxisData<int>& ad)
{
	dbglog << "AXISPROCESSOR GOT: " << ID::asStr(dt)
	       << " ON AXIS: " << ad.numerator
	       << " OF TYPE: " << ID::asStr(at)
	       << " WITH VALUE: " << ad.abs;
}

} // anonymous-namespace

#endif

template <typename Parent>
struct ModFilter : public Parent
{
	explicit ModFilter(typename Parent::EnvT& env) :
		Parent(env),
		mMod(env.mMod),
		mActivated(false)
	{
	}

	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
		if (mMod == code)
		{
			mActivated = (bs == ID::BS_Pressed);
		}
		else if (mMod == 0)
		{
			Parent::FeedButtonData(dt,bs,code);
		}
		else
		{
			if (mActivated)
			{
				Parent::FeedButtonData(dt,bs,code);
			}
		}
	}

	ButtonCodeT mMod;
	bool        mActivated;
};

template <typename Parent>
struct ClickFilter : public Parent
{
	explicit ClickFilter(typename Parent::EnvT& env) :
		Parent(env),
		mFilter(env.mFilter)
	{
	}

	VipPayloadT getResult() const
	{
		return static_cast<s32>(mLastButton);
	}

	virtual void FeedButtonData(ID::DeviceType,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
		if (mFilter == bs ||
		    mFilter == ID::BS_Both)
		{
			mLastButton = code;
			this->Emit();
		}
	}
	
private:
	ID::ButtonState mFilter;
	ButtonCodeT mLastButton;
};

struct NoFeedback
{
	template <typename A, typename B, typename Processor_>
	static void Setup(ID::DeviceType,
	                  A,
	                  B,
	                  Processor_*)
	{
	}
};

struct OnInputFeedback
{
	typedef AxisData<f32> AxisT;

	template <typename Processor_>
	static void Setup(ID::DeviceType,
	                  ID::AxisType,
	                  const AxisData<s32>& data,
	                  Processor_* processor)
	{
		if (data.rel != 0)
			processor->EnableFeedback();
	}
};

struct OnKeyFeedback
{
	template <typename Processor_>
	static void Setup(ID::DeviceType,
	                  ID::ButtonState bs,
	                  ButtonCodeT,
	                  Processor_* processor)
	{
		if (bs == ID::BS_Pressed)
			processor->EnableFeedback();
	}
};

struct WhileKeyFeedback
{
	template <typename Processor_>
	static void Setup(ID::DeviceType,
	                  ID::ButtonState bs,
	                  ButtonCodeT,
	                  Processor_* processor)
	{
		if (bs == ID::BS_Pressed)
		{
			processor->EnableFeedback();
		}
		else if (bs == ID::BS_Released)
		{
			processor->DisableFeedback();
		}
	}
};

//! Helper function for `swinging' the absolute part of an axis back to zero.
//! \param[out] axis The axis to be modified.
//! \param[in] swing Swing is applied stepwise. So this is equal to one step.
//! \return true If swingBack should still be called, false if otherwise.
template <typename AxisT>
bool swingBackAbsolute(AxisT& axis, typename AxisT::ValueT swing)
{
	if (std::abs(axis.abs) > EPSILON_F)
	{
		typename AxisT::ValueT sign = - (axis.abs / std::abs(axis.abs));

		// Don't swing too much (i.e.: from	0.1 to -0.1)
		if (swing > std::abs(axis.abs))
			swing = std::abs(axis.abs);

		axis.abs += swing * sign;
		axis.rel  = swing * sign;
		return true;
	}
	else
	{
		axis.reset();
		return false;
	}
}

//! Helper function for `swinging' the relative part of an axis back to zero.
//! \param[out] axis The axis to be modified.
//! \param[in] swing Swing is applied stepwise. So this is equal to one step.
//! \return true If swingBack should still be called, false if otherwise.
template <typename AxisT>
void swingBackRelative(AxisT& axis, typename AxisT::ValueT swing)
{
	typename AxisT::ValueT contrary_sign = - sign<f32>(axis.rel);

	if (std::abs(axis.rel) < swing)
		swing = std::abs(axis.rel);

	axis.rel += swing * contrary_sign;
}

template <typename Parent>
struct AxisCache : public Parent
{
	using Parent::DisableFeedback;

	typedef AxisData<f32> AxisT;
	typedef typename Parent::EnvT EnvT;

	explicit AxisCache(typename Parent::EnvT& env) :
		Parent(env),
		mSensitivity(env.mSensitivity),
		mFlip(env.mFlip),
		mRange(env.mRange),
		mSwing(env.mSwing)
	{
	}
	
	virtual void onFeedback(long /*microseconds_passed*/)
	{
		/**
			\note
			This swing logic is way better than in swingBack() and
			VirtualAxisWithSwing.
			We need the extra function swingBackRelative() here, because we want
			to decrease the relative part stepwise, not the absolute part. The
			reason for that is the use of relative pixel changes to simulate an
			absolute axis.
			In swingBackAbsolute(), the relative part is always set to the swing
			variable * contrary sign, therefore we wouldn't get any swing at
			all. 
		*/

		bool swinging = (std::abs(mSwing) > EPSILON_F);
		if (swinging)
			swingBackRelative(mAxis, mSwing);

		if (std::abs(mAxis.rel) < EPSILON_F)
		{
			mAxis.rel = 0;
			DisableFeedback();
		}

		this->Emit();

		if (!swinging)
			mAxis.rel = 0;		
	}

	virtual void FeedAxisData(ID::DeviceType dt,
	                          ID::AxisType at,
	                          const AxisData<s32>& ad)
	{
		mAxis = ad;

		if (mFlip)
		{
			mAxis.abs *= -1;
			mAxis.rel *= -1;
		}

		if (mRange > EPSILON_F)
		{
			mAxis.abs /= mRange;
			mAxis.rel /= mRange;
		}

		mAxis.rel *= mSensitivity;

		this->Emit();

		Parent::FeedAxisData(dt,at,ad);
	}

protected:
	AxisT mAxis;
	f32   mSensitivity;
	bool  mFlip;
	f32   mRange;
	f32   mSwing;
};

template
<
	typename AxisFeedback,
	typename ButtonFeedback,
	typename Environment,
	typename Parent = CommonBase
>
class Processor : public Parent
{
public:
	typedef Environment EnvT;

	explicit Processor(EnvT& env) :
		Parent(env.mAction,
		       env.mState,
		       env.mRelevantButtons,
		       env.mRelevantAxis,
		       env.mDevice)
	{
	}
	
	virtual void FeedAxisData(ID::DeviceType dt,
	                          ID::AxisType at,
	                          const AxisData<s32>& ad)
	{
#ifdef CONTROLLER_DEBUG
		AxisDataDebug(dt,at,ad);
#endif
		AxisFeedback::Setup(dt,at,ad,this);
	}

	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
#ifdef CONTROLLER_DEBUG
		ButtonDataDebug(dt,bs,code);
#endif
		ButtonFeedback::Setup(dt,bs,code,this);
	}
};

template <typename Parent>
class VirtualAxis : public Parent
{
public:
	typedef AxisData<float> AxisT;

	explicit VirtualAxis(typename Parent::EnvT& env) :
		Parent(env),
		mLower(env.mLower),
		mRaise(env.mRaise),
		mStep(env.mStep),
		mStopAtZero(env.mStopAtZero)
	{
	}
	
	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
		mButtonCache[code] = (bs == ID::BS_Pressed);
		Parent::FeedButtonData(dt,bs,code);
	}
	
	virtual void onFeedback(long /*microseconds_passed*/)
	{
		if (mButtonCache[mLower])
			lower();
		
		if (mButtonCache[mRaise])
			raise();

		this->Emit();
	}

	void raise()
	{
		//! \todo Make this configurable
		const float mUpperLimit = 1.0f;

		bool wasNegative = mAxis.abs < 0;
	
		mAxis.abs += mStep;
		mAxis.rel  = mStep;

		if (mStopAtZero && mAxis.abs >= 0 && wasNegative)
		{
			Parent::DisableFeedback();
			mAxis.reset();
		}

		if (mAxis.abs > mUpperLimit)
			mAxis.abs = mUpperLimit;
	}
	void lower()
	{
		//! \todo Make this configurable
		const float mLowerLimit = -1.0f;

		bool wasPositive = mAxis.abs > 0;

		mAxis.abs -= mStep;
		mAxis.rel  = -mStep;

		if (mStopAtZero && mAxis.abs <= 0 && wasPositive)
		{
			Parent::DisableFeedback();
			mAxis.reset();
		}

		if (mAxis.abs < mLowerLimit)
			mAxis.abs = mLowerLimit;
	}

protected:
	std::map<ButtonCodeT, bool> mButtonCache;
	
	ButtonCodeT mLower;
	ButtonCodeT mRaise;
	
	float mStep;
	bool  mStopAtZero;

public:
	AxisT mAxis;
};

template <typename Parent>
class VirtualAxisWithSwing : public VirtualAxis<Parent>
{
public:
	using VirtualAxis<Parent>::mAxis;
	using VirtualAxis<Parent>::mLower;
	using VirtualAxis<Parent>::mRaise;
	using VirtualAxis<Parent>::mButtonCache;

	using VirtualAxis<Parent>::lower;
	using VirtualAxis<Parent>::raise;

	explicit VirtualAxisWithSwing(typename Parent::EnvT& env) :
		VirtualAxis<Parent>(env),
		mSwing(env.mSwing)
	{
	}
	
	virtual void onFeedback(long /*microseconds_passed*/)
	{
		if (mButtonCache[mLower])
			lower();
		else
		if (mButtonCache[mRaise])
			raise();
		else
		{
			if (! swingBackAbsolute(mAxis, mSwing))
				Parent::DisableFeedback();
		}

		this->Emit();
	}

	f32 mSwing;
};

template <typename Parent>
class NoResult : public Parent
{
public:
	explicit NoResult(typename Parent::EnvT& env) :
		Parent(env)
	{
	}

	VipPayloadT getResult() const
	{
		return -1;
	}
};

template <typename Parent>
class AxisAsResult : public Parent
{
public:
	typedef typename Parent::AxisT AxisT;
	typedef typename AxisT::ValueT AxisValueT;

	using Parent::mAxis;

	explicit AxisAsResult(typename Parent::EnvT& env) :
		Parent(env),
		mAxisMode(env.mAxisMode)
	{
	}
	
	VipPayloadT getResult() const
	{
		if (mAxisMode == ID::AM_Absolute)
			return mAxis.abs;

		else /* mAxisMode == ID::AM_Relative */
			return mAxis.rel;
	}
	
private:
	ID::AxisMode mAxisMode;
};

template <typename Parent>
class Sequencer : public Parent
{
public:
	explicit Sequencer(typename Parent::EnvT& env) :
		Parent(env),
		mSequence(env.mSequence),
		mFilter(env.mFilter)
	{		
	}
	
	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
		if (mFilter == bs ||
		    mFilter == ID::BS_Both)
		{
			mLastSequence.push_back(std::string::value_type(code));

			while (mLastSequence.length() > mSequence.length())
				mLastSequence.erase(mLastSequence.begin());
					  
			if (mSequence == mLastSequence)
				this->Emit();
		}

		Parent::FeedButtonData(dt,bs,code);
	}
	
private:
	std::string mSequence;
	std::string mLastSequence;
	
	ID::ButtonState mFilter;
};

template <typename Parent>
class Toggler : public Parent
{
public:
	explicit Toggler(typename Parent::EnvT& env) :
		Parent(env),
		mActivated(false),
		mHoldLogic(env.mHold)
	{		
	}
	
	virtual void FeedButtonData(ID::DeviceType dt,
	                            ID::ButtonState bs,
	                            ButtonCodeT code)
	{
		if (mHoldLogic)
			performHoldLogic();
		else
			performSwitchLogic(bs);
	}
	
	VipPayloadT getResult() const
	{
		return mActivated;
	}
	
private:
	void performSwitchLogic(ID::ButtonState bs)
	{
		if (bs == ID::BS_Pressed)
		{
			mActivated = ! mActivated;
			this->Emit();
		}
	}
	
	void performHoldLogic()
	{
		mActivated = ! mActivated;
		this->Emit();
	}

	bool mActivated;   // false = off; true = on
	bool mHoldLogic;
};

} // namespace VIP
} // namespace Controller_
} // namespace BFG

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

#endif
