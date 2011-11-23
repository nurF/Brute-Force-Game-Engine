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

#ifndef COMPOSERACTIONS
#define COMPOSERACTIONS

#include <Core/Types.h>

using namespace BFG;

const s32 A_QUIT = 10000;
const s32 A_LOADING_MESH = 10001;
const s32 A_LOADING_MATERIAL = 10002;
const s32 A_CAMERA_AXIS_X = 10003;
const s32 A_CAMERA_AXIS_Y = 10004;
const s32 A_CAMERA_AXIS_Z = 10005;
const s32 A_CAMERA_MOVE = 10006;
const s32 A_CAMERA_RESET = 10007;
const s32 A_CAMERA_ORBIT = 10008;
const s32 A_SCREENSHOT = 10009;
const s32 A_LOADING_SKY = 10010;

const s32 A_CREATE_LIGHT = 10011;
const s32 A_DESTROY_LIGHT = 10012;
const s32 A_PREV_LIGHT = 10013;
const s32 A_NEXT_LIGHT = 10014;
const s32 A_FIRST_LIGHT = 10015;
const s32 A_LAST_LIGHT = 10016;
const s32 A_INFO_WINDOW = 10017;

const s32 A_CAMERA_MOUSE_X = 10018;
const s32 A_CAMERA_MOUSE_Y = 10019;
const s32 A_CAMERA_MOUSE_Z = 10020;
const s32 A_CAMERA_MOUSE_MOVE = 10021;

const s32 A_SUB_MESH = 10022;
const s32 A_TEX_UNIT = 10023;
const s32 A_ANIMATION = 10024;
const s32 A_ADAPTER = 10025;

const s32 A_MOUSE_MIDDLE_PRESSED = 10026;

const s32 A_UPDATE_FEATURES = 15000;
#endif
