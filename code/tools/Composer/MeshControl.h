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

#ifndef MESHCONTROL
#define MESHCONTROL

#include <BaseFeature.h>

#include <OgreResourceGroupManager.h>

#include <OpenSaveDialog.h>

namespace Tool
{

class MeshControl : public BaseFeature
{
public:
	MeshControl(boost::shared_ptr<SharedData> data, const bool needButton = false) :
	BaseFeature("Mesh", needButton),
	mData(data)
	{
		BFG::Path p;

		mDialog.setDialogInfo
		(
			"Load Mesh",
			"Load",
			 MyGUI::newDelegate(this, &MeshControl::onLoadOk)
		);

		mDialog.setRestrictions
		(
			p.Get(ID::P_GRAPHICS_MESHES),
			true,
			".mesh"
		);
	}

	virtual ~MeshControl()
	{
	}

	virtual void load()
	{
		if (mLoaded)
			return;

		mLoaded = true;
		deactivate();
	}

	virtual void unload()
	{
		if (!mLoaded)
			return;

		if (mActive)
			deactivate();
		
		mLoaded = false;
	}

	virtual void activate()
	{
		mDialog.setVisible(true);
		mActive = true;
	}
	virtual void deactivate()
	{
		mDialog.setVisible(false);
		mActive = false;
	}

	virtual void eventHandler(BFG::Controller_::VipEvent* ve)
	{

	}

protected:

private:
	void onLoadOk(MyGUI::Widget* w)
	{
		std::string folder = mDialog.getCurrentFolder();
		std::string meshName = mDialog.getFileName().substr(folder.size() + 1);

		if (!(mData->mActiveMesh))
		{
			mData->mActiveMesh = generateHandle();
		}

		mData->mRenderObject.reset();
		mData->mRenderObject.reset(new View::RenderObject
		(
			NULL_HANDLE,
			mData->mActiveMesh,
			meshName,
			v3::ZERO,
			qv4::IDENTITY
		));
		mData->mMeshName = meshName;

		Ogre::SceneManager* sceneMgr = Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);
		Ogre::Entity* ent = sceneMgr->getEntity(stringify(mData->mActiveMesh));

		deactivate();
	}

	boost::shared_ptr<SharedData> mData;
	OpenSaveDialog mDialog;
	std::string mPath;
}; // class MeshFeature

} // namespace Tool
#endif