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

#ifndef CTYPES_H
#define CTYPES_H

#include <boost/cstdint.hpp>

#define NULL_HANDLE 0

namespace BFG
{
	// Standard types
	typedef boost::uint8_t    u8;
	typedef boost::int8_t     s8;
	typedef boost::int8_t     c8;
	typedef boost::uint16_t   u16;
	typedef boost::int16_t    s16;
	typedef boost::uint32_t   u32;
	typedef boost::int32_t    s32;

	typedef float             f32;
	typedef double            f64;

	typedef boost::int64_t    GameHandle;
	
	const f32 EPSILON_F = 0.000001f;
}

#endif
