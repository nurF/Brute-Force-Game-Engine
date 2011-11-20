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

#include <View/Camera.h>

#include <OgreHardwarePixelBuffer.h>
#include <OgreMaterialManager.h>
#include <OgreRenderTexture.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <Base/CLogger.h>

#include <Core/Math.h>
#include <Core/Utils.h> // stringify()

#include <View/Convert.h>
#include <View/Defs.h>
#include <View/Event.h>
#include <View/Main.h>


namespace BFG {
namespace View {

Camera::Camera(GameHandle cameraHandle, 
               Ogre::SceneNode* camNode, 
               Ogre::RenderTarget* renderTarget, 
               s32 width, 
               s32 height) :
mRenderTarget(renderTarget),
mCameraNode(camNode),
mNodeCreated(false),
mRenderTargetCreated(false),
mHandle(cameraHandle)
{
	Ogre::SceneManager* sceneMgr;
	if (mCameraNode == NULL) // Create SceneNode
	{
		sceneMgr = Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);
		if (sceneMgr->hasSceneNode(stringify(mHandle)))
		{
			mCameraNode = sceneMgr->getSceneNode(stringify(mHandle));
		}
		else
		{
			mCameraNode = sceneMgr->getRootSceneNode()->createChildSceneNode(stringify(mHandle));
			mNodeCreated = true;
		}
	}
	else
	{
		sceneMgr = mCameraNode->getCreator();
	}
	mCameraNode->resetOrientation();
	mCameraNode->setPosition(toOgre(v3::ZERO));

	v3 target = toBFG(mCameraNode->getOrientation().zAxis());
	norm(target);

	Ogre::Camera* cam;
	cam = sceneMgr->createCamera(stringify(mHandle));

	cam->setFOVy(Ogre::Degree(60.0f));
	cam->setNearClipDistance(0.1f);
	cam->setFarClipDistance(250000.0f);
	cam->lookAt(toOgre(target) * 10);
	mCameraNode->attachObject(cam);

	infolog << "Camera: " << stringify(mHandle) << " created.";

	if (mRenderTarget == NULL)
	{
		// Create renderToTexture RenderTarget

		if (width == 0 || height == 0)
		{
			throw std::logic_error("Too few information to create a render target.");
		}

		cam->setAspectRatio((f32)width / (f32)height);

		Ogre::TexturePtr texture = Ogre::TextureManager::getSingleton().createManual
		(
			stringify(mHandle),
			Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
			Ogre::TEX_TYPE_2D,
			width,
			height,
			0,
			Ogre::PF_R8G8B8,
			Ogre::TU_RENDERTARGET
		);

		mRenderTarget = texture->getBuffer()->getRenderTarget();

		prepareRenderTarget();

		mRenderTarget->addViewport(cam);

		mRenderTarget->getViewport(0)->setClearEveryFrame(true);
		mRenderTarget->getViewport(0)->setBackgroundColour(Ogre::ColourValue::Black);
		mRenderTarget->getViewport(0)->setOverlaysEnabled(false);

		Ogre::MaterialPtr mat =
			Ogre::MaterialManager::getSingleton().getByName(stringify(mHandle));
		if (mat.isNull())
		{
			mat = Ogre::MaterialManager::getSingleton().create(
			    stringify(mHandle), 
			    Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
		}
		Ogre::Technique* tech = mat->getTechnique(0);
		if (!tech)
		{
			tech = mat->createTechnique();
		}
		Ogre::Pass* pass = tech->getPass(0);
		if (!pass)
		{
			pass = tech->createPass();
		}
		pass->setLightingEnabled(false);

		Ogre::TextureUnitState* txState = NULL;
		if (pass->getNumTextureUnitStates() > 0)
		{
			txState = pass->getTextureUnitState(0);
			txState->setTextureName(stringify(mHandle));
		}
		else
		{
			pass->createTextureUnitState(stringify(mHandle));
		}

		mRenderTarget->setAutoUpdated(true);

		mRenderTargetCreated = true;

		infolog << "Rendertarget: " << stringify(mHandle) << " created.";
	}
	else
	{
		prepareRenderTarget();
		f32 width = mRenderTarget->getWidth();
		f32 height = mRenderTarget->getHeight();
		cam->setAspectRatio(width / height);
		mRenderTarget->addViewport(cam);
	}
	
	Main::eventLoop()->connect
	(
		ID::VE_UPDATE_POSITION,
		this,
		&Camera::viewEventHandler,
		mHandle
	);
	
	Main::eventLoop()->connect
	(
		ID::VE_UPDATE_ORIENTATION,
		this,
		&Camera::viewEventHandler,
		mHandle
	);

}

void Camera::prepareRenderTarget()
{
	unsigned short numVp = mRenderTarget->getNumViewports();
	if (numVp > 0)
	{
		mRenderTarget->removeViewport(0);
	}
}
Camera::~Camera()
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr = root.getSceneManager(BFG_SCENEMANAGER);
	
	Main::eventLoop()->disconnect(ID::VE_UPDATE_POSITION, this);
	Main::eventLoop()->disconnect(ID::VE_UPDATE_ORIENTATION, this);

	if (mRenderTargetCreated)
	{
		Ogre::TextureManager::getSingleton().remove(stringify(mHandle));
		Ogre::MaterialManager::getSingleton().remove(stringify(mHandle));
		mRenderTarget = NULL;
	}

	sceneMgr->destroyCamera(stringify(mHandle));

	if (mNodeCreated)
	{
		sceneMgr->destroySceneNode(mCameraNode);
	}
}

void Camera::updatePosition(const v3& pos)
{
	mCameraNode->setPosition(toOgre(pos));
}

void Camera::updateOrientation(const qv4& ori)
{
	mCameraNode->setOrientation(toOgre(ori));
}

void Camera::toggleWireframe()
{
	Ogre::Camera* cam = mCameraNode->getCreator()->getCamera(stringify(mHandle));
	Ogre::PolygonMode mode = cam->getPolygonMode();
	if (mode == Ogre::PM_WIREFRAME)
	{
		cam->setPolygonMode(Ogre::PM_SOLID);
	} 
	else
	{
		cam->setPolygonMode(Ogre::PM_WIREFRAME);
	}
}

void Camera::viewEventHandler(Event* VE)
{
	switch (VE->getId())
	{
	case ID::VE_UPDATE_POSITION:
		updatePosition(boost::get<v3>(VE->getData()));
		break;
	case ID::VE_UPDATE_ORIENTATION:
		updateOrientation(boost::get<qv4>(VE->getData()));
		break;
	default:
		throw std::logic_error("Camera::eventHandler: received unhandled event!");
	}

}
} // namespcae View
} // namespace BFG
