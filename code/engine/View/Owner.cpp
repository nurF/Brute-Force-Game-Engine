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

#include <View/Owner.h>

#include <OgreRoot.h>
#include <OgreRenderWindow.h>

#include <MyGUI.h>

#include <EventSystem/Core/EventLoop.h>
#include <Core/Utils.h>

#include <View/Event.h>

#include <View/Camera.h>
#include <View/RenderObject.h>
#include <View/Skybox.h>

namespace BFG {
namespace View {

Owner::Owner(GameHandle stateHandle, EventLoop* loop) :
mLoop(loop)
{
	//! \note
	//! VE_DESTROY_OBJECT makes no use of stateHandle
	loop->connect(ID::VE_CREATE_OBJECT, this, &Owner::eventHandler, stateHandle);
	loop->connect(ID::VE_DESTROY_OBJECT, this, &Owner::eventHandler);
	loop->connect(ID::VE_CREATE_CAMERA, this, &Owner::eventHandler, stateHandle);
	loop->connect(ID::VE_SET_SKY, this, &Owner::eventHandler, stateHandle);
}

Owner::~Owner()
{
	mLoop->disconnect(ID::VE_CREATE_OBJECT, this);
	mLoop->disconnect(ID::VE_DESTROY_OBJECT, this);
	mLoop->disconnect(ID::VE_CREATE_CAMERA, this);
	mLoop->disconnect(ID::VE_SET_SKY, this);
}

void Owner::eventHandler(Event* VE)
{
	switch(VE->getId())
	{
	case ID::VE_CREATE_OBJECT:
		createObject(boost::get<ObjectCreation>(VE->getData()));
		break;

	case ID::VE_DESTROY_OBJECT:
		destroyObject(boost::get<GameHandle>(VE->getData()));
		break;

	case ID::VE_CREATE_CAMERA:
		createCamera(boost::get<CameraCreation>(VE->getData()));
		break;

	case ID::VE_SET_SKY:
		setSky(boost::get<View::SkyCreation>(VE->getData()));
		break;

	default:
		throw std::logic_error("Unhandled event at Owner::eventHandler!");
	}
}

void Owner::createObject(ObjectCreation& OC)
{
	boost::shared_ptr<RenderObject> ro;
	ro.reset(new RenderObject
	(
		OC.mParent,
		OC.mHandle,
		OC.mMeshName.c_array(),
		OC.mPosition,
		OC.mOrientation
	));
	mObjects[OC.mHandle] = ro;
}

void Owner::destroyObject(GameHandle handle)
{
	ObjectMapT::iterator objIt = mObjects.find(handle);
	if (objIt != mObjects.end())
	{
		mObjects.erase(objIt);
	}
}

void Owner::createCamera(CameraCreation& CC)
{
	Ogre::Root& root = Ogre::Root::getSingleton();

	Ogre::SceneNode* node = nullptr;
	Ogre::RenderTarget* target = nullptr;

	if (CC.mNodeHandle != NULL_HANDLE)
	{
		Ogre::SceneManager* sceneMgr =
			root.getSceneManager(BFG_SCENEMANAGER);

		node = sceneMgr->getSceneNode(stringify(CC.mNodeHandle));
	}
	if (CC.mFullscreen == true)
	{
		target = root.getAutoCreatedWindow();
	}
	boost::shared_ptr<Camera> cam;
	cam.reset(new Camera
	(
		CC.mHandle,
		node,
		target,
		CC.mWidth,
		CC.mHeight
	));
	mCameras.push_back(cam);
}

void Owner::setSky(SkyCreation& SC)
{
	mSky.reset();   // fix (skybox dtor must be called explicitly) 
	mSky.reset(new Skybox(SC.mMatName.c_array()));
}

} // namespace View
} // namespace BFG
