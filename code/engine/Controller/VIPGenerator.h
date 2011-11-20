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

#ifndef _VIPGENERATOR_H_
#define _VIPGENERATOR_H_

#include <boost/mpl/vector.hpp>

#include <Controller/VIPBase.h>
#include <Controller/VIPComponents.h>

namespace BFG {
namespace Controller_ {
namespace VIP {

/** \file

	\verbatim
	
    So you want to add new stuff? (r684)
   ---------------------------------------

	>> VIPS <<

	VIPs are Virtual Input Processors. That is, they take simple data
	and do something fancy with it. Then, they send the result to EventCore.
   
   1) Add an identifier to `enum VIPType' (in "ControllerEnums.xml")
   2) Create/extend/modify components if necessary
   3) Create typedef (in this file)
   4) Add that typedef to VIP::ALL_TYPES (also in this file)
   5) Add entry in VIP::Factory::Create()
   6) Bind VIP in some state file. (e.g. in gamePlayStateCfg.lua)
   7) Add the new VIP to *all* emitters and `EmitterVisitorBase'

	>> Environments <<

	Environments contain important data for VIPs.
	Most of it is delivered by Lua atm.

   1) Create/extend/modify environments if necessary
   2) Add a Init() function in VIPLuaInitializer.{h,cpp}
   3) Check EnvT Parameter for Processor<>
   4) Check if all variables of the new environment get
      copied to the components.

	>> New VIP Config Parameter <<
	
   1) Use it in your VIP component. Add it as member AND as ctor argument,
      don't forget ctor list initialization. (VIPComponents.h)
   2) Add it to the environment used by your component (VIPEnvironment.h)
   3) Add initializer code for your variable in the particular environment
      (VIPLuaInitializer.cpp)
   4) Use the new parameter in the desired Lua files.

    Additional Notes
   ------------------
	- Inherit at least from `CommonBase' or `Processor', because they define
	  important type traits which define the T::Feed____Data() interface for
	  `class Delegater' and further VIP related components.

	- Deepest/First layer needs to be `Vip'
	  
	- If you need "feedback", don't overwrite CommonBase::FeedTime() directly.
	  Use the FeedbackTrigger or your own stuff and overload onFeedback()
	  then.

	\endverbatim
*/

typedef
Vip <
	ID::VIP_CLICK,
	ClickFilter <
		ModFilter <
			Processor <NoFeedback, NoFeedback, ModClickEnv>
		>
	>
> Click;

typedef
Vip <
	ID::VIP_STEER,
	AxisAsResult <
		AxisCache <
			FeedbackTrigger <
				Processor <OnInputFeedback, NoFeedback, SteerEnv>
			>
		>
	>
> Steer;

typedef
Vip <
	ID::VIP_HOLD,
	NoResult <
		Repeater <
			FeedbackTrigger <
				Processor <NoFeedback, WhileKeyFeedback, FeedbackEnv>
			>
		>
	>
> Hold;

typedef
Vip <
	ID::VIP_THRUST_AXIS,
	AxisAsResult <
		VirtualAxis <
			FeedbackTrigger <
				Processor <NoFeedback, WhileKeyFeedback, VAxisEnv>
			>
		>
	>
> ThrustAxis;

typedef
Vip <
	ID::VIP_LINEAR_AXIS,
	AxisAsResult <
		VirtualAxisWithSwing <
			FeedbackTrigger <
				Processor <NoFeedback, OnKeyFeedback, VAxisSwingEnv>
			>
		>
	>
> LinearAxis;

typedef
Vip <
	ID::VIP_CHEAT,
	NoResult <
		Sequencer <
			Processor <NoFeedback, NoFeedback, SequenceEnv>
		>
	>
> Cheat;

typedef
Vip <
	ID::VIP_TOGGLE,
	Toggler <
		Processor <NoFeedback, NoFeedback, ToggleEnv>
	>
> Toggle;

// Observations
// ============
//
// * LinearAxis uses almost the same initalization code as ThrustAxis
//
// * If you press the Keyboard button and the Mouse button for "FIRE"
//   at the same time, two events get triggered. Not a bug but undesired
//   behaviour.
//
// * Combine ClickFilter and Sequencer? (Both use the ClickEnv)
//
// * More warnings necessary on missing lua parameters
//
// * Combine assignDevice() and assignDefault?
// ...........................................................................

//! Important: Don't forget to add new types to this vector!
typedef boost::mpl::vector<
	Click,
	Steer,
	Hold,
	ThrustAxis,
	LinearAxis,
	Cheat,
	Toggle
> ALL_TYPES;

} // namespace VIP
} // namespace Controller_
} // namespace BFG

#endif
