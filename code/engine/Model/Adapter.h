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

#ifndef BFG_ADAPTER__
#define BFG_ADAPTER__

#include <Core/Types.h>
#include <Core/v3.h>
#include <Core/qv4.h>

namespace BFG {

/** \brief
		Simple connection between Modules

	Used to determine if there is a connection between modules possible or
	if there is allready one.
*/
struct Adapter
{
	u32 mIdentifier;
	/// The local orientation
	qv4 mParentOrientation;
	qv4 mChildOrientation;
	
	/// The local positions
	v3 mParentPosition;
	v3 mChildPosition;

	Adapter() :
	mIdentifier(0),
	mParentOrientation(qv4::IDENTITY),
	mChildOrientation(qv4::IDENTITY),
	mParentPosition(v3::ZERO),
	mChildPosition(v3::ZERO)
	{
	}
	
	bool good() const
	{
		return mIdentifier > 0;
	}
};

} // namespace BFG

#endif