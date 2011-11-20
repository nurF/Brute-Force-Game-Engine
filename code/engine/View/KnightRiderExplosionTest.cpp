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

#include <View/KnightRiderExplosionTest.h>

#include <boost/bind.hpp>

#include <Base/Cpp.h>
#include <View/Effect.h>
#include <View/Explosion.h>

namespace BFG {
namespace View {

void KnightRiderExplosionTest::update(f32 timeSinceLastFrame)
{
	erase_if(all, boost::bind(&Effect::done, _1));

	static f32 time = 0.0f;

	time += timeSinceLastFrame;

	static f32 x = 0.0f;
	
	if (time > 0.1f)
	{
		x += 0.5f;
	
		boost::shared_ptr<ExpT> e(new Explosion2(v3(cos(x) * 100.0f, 0.0f, 200.0f), 1.0f));
		all.push_back(e);

		time = 0.0f;
	}
}

} // namespace View
} // namespace BFG