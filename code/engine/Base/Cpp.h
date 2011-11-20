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

#ifndef BFG_BASE_CPP_H_
#define BFG_BASE_CPP_H_

#include <algorithm>

//! Erase elements fulfilling condition
template<class C, class P>
void erase_if(C& c, P pred)
{
	c.erase(std::remove_if(c.begin(), c.end(), pred), c.end());
}

//! Applies f to each dereferenced iterator i in the 
//! range [first, last) where: pred(*i) != false
template<class InputIterator, class Function, class Predicate>
Function for_each_if(InputIterator first, 
                         InputIterator last, 
                         Predicate pred, 
                         Function f)
{
	for(; first != last; ++first)
	{
		if( pred(*first) )
			f(*first);
	}
	return f;
};

//! This is a kind of lossless two-way cast.
//! For example, you may convert float(0.123f) to int(1039918957) so you
//! can re-convert it back later to float(0.123f). A normal cast would yield
//! int(0). Try to avoid this function and use boost::variant instead.
template <typename T, typename U>
T falk_cast(U u)
{
	return *(T*)&u;
}

//! Utility for boost::shared_ptr.
struct null_deleter
{
    void operator()(void const *) const
    {
    }
};

#endif
