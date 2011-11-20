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

#ifndef WAYPOINT_H_
#define WAYPOINT_H_

#include <vector>

#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/v3.h>
#include <Core/qv4.h>
#include <Core/Types.h>
#include <Core/Location.h>

#include <Model/Managed.h>

namespace BFG {

class WaypointEvent;

struct MODEL_API Waypoint : public Managed, boost::noncopyable
{
	typedef std::vector<boost::shared_ptr<Waypoint>	> ContainerT;


	Waypoint(GameHandle handle,
	         const Location& location,
	         GameHandle owner);

	~Waypoint();

	Location   mLocation;
	GameHandle mOwner;

private:
	void onChangePosition(const v3& position);
	void onChangeOrientation(const qv4& orientation);
	
	virtual void internalUpdate(quantity<si::time, f32> timeSinceLastFrame);
};

} // namespace BFG

#endif