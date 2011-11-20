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

#ifndef _BFGAXIS_H_
#define _BFGAXIS_H_

namespace BFG {
namespace Controller_ {

template <int v>
struct Int2Type
{
	enum { value = v }; 
};

typedef int AxisNumeratorT;

template <typename T>
struct AxisData
{
	typedef T ValueT;

    //! Damn, we need it.
    AxisData() :
            abs(0),
            rel(0),
            numerator(0)
    {
    }

	explicit AxisData(AxisNumeratorT numerator) :
		abs(0), rel(0), numerator(numerator)
	{
	}

	AxisData(T abs, T rel, AxisNumeratorT numerator) :
		abs(abs), rel(rel), numerator(numerator)
	{
	}
	
	void reset()
	{
		abs = 0;
		rel = 0;
		numerator = 0;
	}

	AxisData<float>& operator = (const AxisData<int>& rhs)
	{
		abs       = static_cast<float>(rhs.abs);
		rel       = static_cast<float>(rhs.rel);
		numerator = rhs.numerator;
		return *this;
	}

	T abs;
	T rel;

	//! This is the axis identifier, or axis enumerator. A mouse has two
	//! axis, whereas a joystick may have five or even more.
	AxisNumeratorT numerator;
};

} // namespace Controller_
} // namespace BFG

#endif