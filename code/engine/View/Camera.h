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

#ifndef BFG_CAMERA_H
#define BFG_CAMERA_H

#include <string>

#include <Core/qv4.h>
#include <Core/Types.h>
#include <Core/v3.h>

#include <View/Defs.h>
#include <View/Event_fwd.h>

namespace Ogre
{
class SceneNode;
class RenderTarget;
}

namespace BFG {
namespace View {

class VIEW_API Camera
{
public:
	Camera(GameHandle cameraHandle, 
	       Ogre::SceneNode* camNode = NULL,
	       Ogre::RenderTarget* renderTarget = NULL,
	       s32 width = 0,
	       s32 height = 0);

	virtual ~Camera();

	void toggleWireframe();

	void updatePosition(const v3& pos);
	void updateOrientation(const qv4& ori);

private:
	void viewEventHandler(Event* VE);

	void prepareRenderTarget();


	Ogre::SceneNode* mCameraNode;
	Ogre::RenderTarget* mRenderTarget;
	
	GameHandle mHandle;
	bool mNodeCreated;
	bool mRenderTargetCreated;

}; // class Camera

} // namespace View
} // namespace BFG

#endif
