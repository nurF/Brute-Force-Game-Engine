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

#ifndef MESHFEATURE
#define MESHFEATURE

#include <Core/Math.h>
#include <Model/GameObject.h>
#include <Model/Environment.h>
#include <View/CameraCreation.h>
#include <View/Owner.h>

#include <Actions.h>
#include <BaseFeature.h>
#include <Event_fwd.h>
#include <SharedData.h>

namespace Tool
{

class CameraControl : public BaseFeature, public Emitter
{
public:
	CameraControl(EventLoop* loop, boost::shared_ptr<SharedData> data) :
	BaseFeature("CameraControl", false),
	Emitter(loop),
	mData(data),
	mCameraPosition(NULL),
	mCameraRotation(NULL),
	mCameraDistance(NULL),
	mDeltaRot(v3::ZERO),
	mDeltaDis(0.0f),
	mCamDistance(0.0f),
	mCamOrbit(false),
	mMouseCamPitchYaw(false),
	mMouseCamRoll(false),
	mIsZooming(false)
	{
		createDefaultCamera();
	}

	virtual ~CameraControl()
	{

	}

	virtual void load(){mLoaded = true;}
	virtual void unload(){mLoaded = false;}

	virtual void activate()
	{
		mActive = true;
		emit<Tool::Event>(A_UPDATE_FEATURES, 0);
	}
	virtual void deactivate()
	{
		mActive = false;
		emit<Tool::Event>(A_UPDATE_FEATURES, 0);
	}

	void createDefaultCamera()
	{
		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		mCameraPosition = sceneMgr->getRootSceneNode()->createChildSceneNode();
		mCameraPosition->setPosition(0, 0, 0);

		mCameraRotation = mCameraPosition->createChildSceneNode();

		mCameraDistance = mCameraRotation->createChildSceneNode(stringify(mData->mCamera));

		View::CameraCreation cc
		(
			mData->mCamera,
			mData->mCamera,
			true,
			0,
			0
		);
		
		emit<View::Event>(ID::VE_CREATE_CAMERA, cc, mData->mState);

		Ogre::Vector3 pos(0.0f, 0.0f, -mCamDistance);
		mCameraDistance->setPosition(pos);
	}

	void eventHandler(BFG::Controller_::VipEvent* ve)
	{
		switch(ve->getId())
		{
		case A_CAMERA_AXIS_X:
		{
			onCamX(boost::get<f32>(ve->getData()));
			break;
		}
		case A_CAMERA_MOUSE_X:
		{
			if (mMouseCamPitchYaw)
			{
				onCamX(boost::get<f32>(ve->getData()));
			}
			break;
		}
		case A_CAMERA_AXIS_Y:
		{
			onCamY(boost::get<f32>(ve->getData()));
			break;
		}
		case A_CAMERA_MOUSE_Y:
		{
			if (mMouseCamPitchYaw)
			{
				onCamY(boost::get<f32>(ve->getData()));
			}
			break;
		}
		case A_CAMERA_AXIS_Z:
		{
			onCamZ(boost::get<f32>(ve->getData()));
			break;
		}
		case A_CAMERA_MOUSE_Z:
		{
			if (mMouseCamRoll)
			{
				onCamZ(boost::get<f32>(ve->getData()));
			}
			break;
		}
		case A_CAMERA_MOVE:
		{
			f32 value = boost::get<f32>(ve->getData());
			if (value > EPSILON_F || value < -EPSILON_F)
			{
				mIsZooming = true;
				mDeltaDis += value;
			}
			else
			{
				mIsZooming = false;
				mDeltaDis = 0.0f;
			}
			break;
		}
		case A_CAMERA_MOUSE_MOVE:
		{
			mIsZooming = false;
			mDeltaDis = boost::get<f32>(ve->getData());
			break;
		}
		case A_CAMERA_RESET:
		{
			onReset();
			break;
		}
		case A_CAMERA_ORBIT:
		{
			mCamOrbit = boost::get<bool>(ve->getData());
			break;
		}
		case A_MOUSE_MIDDLE_PRESSED:
		{
			if ( boost::get<bool>(ve->getData()) )
				mMouseCamPitchYaw = true;
			else
				mMouseCamPitchYaw = false;
			
			break;
		}
		case BFG::ID::A_MOUSE_RIGHT_PRESSED:
		{
			if ( boost::get<bool>(ve->getData()) )
				mMouseCamRoll = true;
			else
				mMouseCamRoll = false;
			break;
		}
		}
	}

	virtual void update(const Ogre::FrameEvent& evt)
	{
		if (mCamOrbit)
		{
			mCameraRotation->rotate
			(
				Ogre::Vector3::UNIT_Y,
				Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame,
				Ogre::Node::TS_WORLD
			);

			mCameraRotation->rotate
			(
				Ogre::Vector3::UNIT_X,
				Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame,
				Ogre::Node::TS_WORLD
			);
		}
		else
		{
			mCameraRotation->yaw(Ogre::Radian(mDeltaRot.y) * evt.timeSinceLastFrame);
			mCameraRotation->pitch(Ogre::Radian(mDeltaRot.x) * evt.timeSinceLastFrame);
			mCameraRotation->roll(Ogre::Radian(mDeltaRot.z) * evt.timeSinceLastFrame);
		}

		mCamDistance += mDeltaDis * evt.timeSinceLastFrame;
		if (mCamDistance <= 1.0f)
			mCamDistance = 1.0f;

		mCameraDistance->setPosition(0.0f, 0.0f, -mCamDistance);

		mDeltaRot = v3::ZERO;

		if (!mIsZooming)
		{
			mDeltaDis = 0.0f;
		}

	}
protected:

private:

	void onCamX(f32 x)
	{
		mDeltaRot.x = M_PI * x;
	}

	void onCamY(f32 y)
	{
		mDeltaRot.y = M_PI * y;
	}

	void onCamZ(f32 z)
	{
		mDeltaRot.z = M_PI * z;
	}

	void onReset()
	{
		mCameraRotation->setOrientation(Ogre::Quaternion::IDENTITY);
	}

	boost::shared_ptr<SharedData> mData;

	Ogre::SceneNode* mCameraPosition;
	Ogre::SceneNode* mCameraRotation;
	Ogre::SceneNode* mCameraDistance;

	v3 mDeltaRot;
	f32 mDeltaDis;
	f32 mCamDistance;
	bool mCamOrbit;
	bool mMouseCamPitchYaw;
	bool mMouseCamRoll;
	bool mIsZooming;
}; // class MeshFeature

} // namespace Tool
#endif