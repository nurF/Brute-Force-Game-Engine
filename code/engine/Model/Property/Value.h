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

#ifndef BFG_PROPERTYVALUE_H__
#define BFG_PROPERTYVALUE_H__

#include <string>
#include <sstream>
#include <Model/Events/GameObjectEvent_fwd.h>

#include <Model/Defs.h>

namespace BFG {
namespace Property {

class MODEL_API Value
{
public:
	Value()
	{
	}
	
	Value(const Value& rhs) :
	mValue(rhs.mValue)
	{
	}
	
	template <typename T>
	Value(const T& rhs) :
	mValue(rhs)
	{
	}

	template <typename T>
	operator T&()
	{
		return boost::get<T>(mValue);
	}
	
	template <typename T>
	operator const T&() const
	{
		return boost::get<T>(mValue);
	}
	
	template <typename T>
	bool operator == (const T& rhs) const
	{
		return boost::get<T>(mValue) == rhs;
	}
	
	template <typename T>
	bool operator > (const T& rhs) const
	{
		return boost::get<T>(mValue) > rhs;
	}
	
	//! Might conflict with boost::units::operator * and become ambigous.
	//! If this is the case, perform a static cast or just change the order
	//! FROM: var * unit
	//! TO: unit * var
	template <typename T>
	T operator * (const T& rhs) const
	{
		return boost::get<T>(mValue) * rhs;
	}

#if DEACTIVATED_DUE_TO_INFINITE_RECURSION_BETWEEN_THESE_TWO_FUNCTIONS
	std::string string() const
	{
		std::string result;
		std::stringstream ss;
		ss << mValue;
		result = ss.str();
		return result;
	}

	friend std::ostream& operator<< (std::ostream& lhs, const Value& rhs)
	{
		if (rhs.mValue.type() == typeid(Value))
			return lhs;
	
		lhs << rhs.string();
		return lhs;
	}
#endif

private:
	GoePayloadT mValue;
};

} // namespace Property
} // namespace BFG

#endif
