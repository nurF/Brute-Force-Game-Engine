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

#include "CVersion.h"
#include <sstream>

namespace BFG {

std::string CVersion::toString(const std::string& sep) const
{
	std::stringstream str;
	str << SMajor << sep
		<< Major  << sep
		<< Minor;

	return str.str();
}

void CVersion::init(int smaj, int maj, int min)
{
	SMajor = smaj;
	Major = maj;
	Minor = min;
}

bool CVersion::operator == (const CVersion& rhs) const
{
	return SMajor == rhs.SMajor
		&& Major  == rhs.Major
		&& Minor  == rhs.Minor;
}

bool CVersion::operator < (const CVersion& rhs) const
{
	if (SMajor < rhs.SMajor)
		return true;
	else if (Major < rhs.Major)
		return true;
	else if (Minor < rhs.Minor)
		return true;
	return false;
}

std::ostream& operator << (std::ostream& lhs, const CVersion& rhs)
{
	lhs << rhs.toString();
	return lhs;
}

} // namespace BFG