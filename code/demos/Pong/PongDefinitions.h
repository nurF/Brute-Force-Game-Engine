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

#ifndef __PONG_DEFINITIONS_H_
#define __PONG_DEFINITIONS_H_

#include <Core/v3.h>
#include <Core/Types.h>

#define PV_DESIRED_SPEED 200

#define A_UPPER_BAR_WIN 10001
#define A_LOWER_BAR_WIN 10002

const BFG::s32 A_SHIP_AXIS_Y = 10000;
const BFG::s32 A_SHIP_FIRE   = 10001;
const BFG::s32 A_QUIT        = 10002;
const BFG::s32 A_FPS         = 10003;

const BFG::f32 OBJECT_Z_POSITION = 25.0f;
const BFG::f32 SPECIAL_PACKER_MESH_OFFSET = 4.0f;
const BFG::f32 DISTANCE_TO_WALL = 17.0f;
const BFG::f32 BAR_Y_POSITION = 10.0f;

const BFG::f32 DESIRED_SPEED = 15.0f;
const BFG::v3 BALL_START_VELOCITY(0.0f, -DESIRED_SPEED, 0.0f);

#endif //__PONG_DEFINITIONS_H_