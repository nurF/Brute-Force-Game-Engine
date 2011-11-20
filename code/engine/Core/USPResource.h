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

#ifndef BFG_RESOURCE_H__
#define BFG_RESOURCE_H__

namespace BFG
{

class ResourceHandle
{
	typedef InternalType long long;
public:
	ResourceHandle(){}
	~ResourceHandle(){}

	void * asPtr() const { return (void*) m_Value; }
	
	template class<T> as<T>() const { return (T) m_Value; }
	template class<T> asPtr<T>() const { return (T*) m_Value; }

	template class<T> void set<T>(T t) { m_Value = t; }
private:
	/// need native 8-bytes type
	/// our resource-Type
	InternalType m_Type;
	/// the value of this resourceHandle, normally a pointer
	InternalType m_Value;
};


}//namespace


#endif // BFG_RESOURCE_H__
