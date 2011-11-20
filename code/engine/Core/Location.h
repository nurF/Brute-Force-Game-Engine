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

#ifndef BFG_LOCATION_H
#define BFG_LOCATION_H

#include <Core/v3.h>
#include <Core/qv4.h>

namespace BFG {

struct Location
{
	Location(const v3& position = v3::ZERO,
	         const qv4& orientation = qv4::IDENTITY) :
		position(position),
		orientation(orientation) {}

	Location(const Location& location) :
		position(location.position),
		orientation(location.orientation) {}

	std::string string() const
	{
		std::string result;
		std::stringstream ss;
		ss << "Pos: " << position
		   << " Ori: " << orientation;
		result = ss.str();
		return result;
	}
	
	friend std::ostream& operator<< (std::ostream& lhs, const Location& rhs)
	{
		lhs << rhs.string();
		return lhs;
	}
	
	v3 position;
	qv4 orientation;
};

} // namespace BFG

#endif
