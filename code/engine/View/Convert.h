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

#ifndef BFG_CONVERT_H
#define BFG_CONVERT_H

#include <Core/v3.h>
#include <Core/qv4.h>

#include <OgreVector3.h>
#include <OgreQuaternion.h>

#include <View/Defs.h>


namespace BFG {
namespace View {

VIEW_API Ogre::Vector3 toOgre(const v3& vec);

VIEW_API Ogre::Quaternion toOgre(const qv4& quat);

VIEW_API v3 toBFG(const Ogre::Vector3& vec);

VIEW_API qv4 toBFG(const Ogre::Quaternion& quat);

} // namespace View
} // namespace BFG

#endif