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

#ifndef BFG_PATH__
#define BFG_PATH__

#include <map>
#include <string>
#include <vector>

#include <Core/Enums.hh>
#include <Core/Defs.h>

#ifdef _MSC_VER
  #pragma warning (push)
  #pragma warning (disable:4251) // "'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'"
#endif

namespace BFG {

struct BFG_CORE_API PathProperties 
{
	PathProperties(const std::string& location, bool autocreate) :
	mLocation(location),
	mAutocreate(autocreate){}
	
	std::string mLocation;
	bool mAutocreate;
};

//! \todo At the moment, path values are verified, but not modified.
//!       That is, they might be relative or absolute.
//!       Use Boost.Filesystem to convert these values into absolute
//!       paths regardless if they're relative or not. 

struct BFG_CORE_API Path
{

	typedef std::map<ID::PathType, PathProperties> ContainerT;

	Path();

	//! Get the relative path of any file within search range.
	/**
		\param filename       Name of searched file
		\return               The relative path plus file name
		\throw std::exception If the file wasn't found
	*/ 
	std::string Expand(const std::string& filename) const;

	//! Get the corresponding relative path to its enum identifier
	//! counterpart.
	/**
		\param pt             Enum value; see Enums.xml
		\return               Just what you requested: The relative path
		\throw std::exception If there is no path for this enumeration
		                      value.
	*/
	std::string Get(ID::PathType pt) const;
	
private:
	template <typename Loader>
	void load();

	void checkPaths() const;

	ID::PathType search(const std::string& filename) const;

	ContainerT mAll;
};

/**
	Puts all level directories which contain Sector.xml (except "default" and
	hidden ones) into "result".

	\note P_SCRIPTS_LEVELS is used as search path.

	\param[out] result Will be filled with level names (which are also the
	                   names of their directories).
*/
void BFG_CORE_API levelDirectories(std::vector<std::string>& result);


} // namespace BFG

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

#endif
