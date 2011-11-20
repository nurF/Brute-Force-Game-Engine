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

#ifndef _DELEGATER_H_
#define _DELEGATER_H_

#include <boost/noncopyable.hpp>

#include <Controller/VIPBase.h>

namespace BFG {
namespace Controller_ {

class Delegater : boost::noncopyable
{
public:
	// Button VIPs
	void registerVIP(ButtonCodeT code,
	                 VipPtrT vip)
	{
		mButtonVIPs.insert(std::make_pair(code, vip));
	}

	// Axis VIPs
	void registerVIP(ID::AxisType at,
	                 AxisNumeratorT axis_numerator,
	                 VipPtrT vip)
	{
		assert (at == ID::AT_Normal &&
		        "Only normal axes are implemented atm.");
		
		mNormalAxisVIPs.insert(std::make_pair(axis_numerator, vip));
	}

	void Delegate(ID::DeviceType dt,
	              ID::ButtonState bs,
	              ButtonCodeT code) const
	{
		typedef ButtonProcessorContainerT::const_iterator Iter;
		typedef std::pair<Iter,Iter> RangeT;
		
		RangeT Range = mButtonVIPs.equal_range(code);

		Iter it = Range.first;

		for(; it != Range.second; ++it)
			it->second->FeedButtonData(dt, bs, code);
		
		// Now again for KB_ANY VIPs.
		Range = mButtonVIPs.equal_range(ID::KB_ANY);
		it = Range.first;
		
		for (; it != Range.second; ++it)
			it->second->FeedButtonData(dt, bs, code);
	}

	template <typename T>
	void Delegate(ID::DeviceType dt,
	              ID::AxisType at,
	              const AxisData<T>& ad) const
	{
		assert (at == ID::AT_Normal &&
		        "Only normal axes are implemented atm.");

		typedef AxisProcessorContainerT::const_iterator Iter;
		typedef std::pair<Iter,Iter> RangeT;
		
		RangeT Range = mNormalAxisVIPs.equal_range(ad.numerator);

		Iter it = Range.first;

		for(; it != Range.second; ++it)
			it->second->FeedAxisData(dt, at, ad);
	}

private:
	typedef std::multimap<ButtonCodeT, VipPtrT> ButtonProcessorContainerT;
	typedef std::multimap<AxisNumeratorT, VipPtrT> AxisProcessorContainerT;

	ButtonProcessorContainerT mButtonVIPs;
	AxisProcessorContainerT   mNormalAxisVIPs;
	
	//AxisProcessorContainerT mSliderAxisVIPs;
	//AxisProcessorContainerT mPovAxisVIPs;
};

} // namespace Controller_
} // namespace BFG

#endif
