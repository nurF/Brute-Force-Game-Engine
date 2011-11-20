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

#ifndef BFG_MANAGED__
#define BFG_MANAGED__

#include <boost/units/systems/si/time.hpp>
#include <boost/units/quantity.hpp>

#include <Core/Types.h>
#include <Model/Enums.hh>
#include <Model/Defs.h>

using namespace boost::units;

namespace BFG {

class MODEL_API Managed
{
public:
	Managed(GameHandle handle,
	        const std::string& publicName,
	        ID::ObjectType type);

	virtual ~Managed();

	GameHandle         getHandle() const {return mHandle;}
	const std::string& getPublicName() const;

	ID::ObjectType getObjectType() const {return mType;}

	void update(quantity<si::time, f32> timeSinceLastFrame);

protected:
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame) = 0;

	// It's the ID for the Managed at the moment.
	GameHandle mHandle;

	// A name that is in a shape to be print in the front-end.
	std::string mPublicName;

	ID::ObjectType mType;
};

} // namespace BFG

#endif
