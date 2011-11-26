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

#ifndef MATERIALCHANGE
#define MATERIALCHANGE

#include <BaseFeature.h>

#include <OgreMaterialManager.h>
#include <OgreSceneManager.h>

#include <SharedData.h>

namespace Tool
{

class MaterialChange : public BaseFeature
{
public:
	MaterialChange(boost::shared_ptr<SharedData> data) :
	BaseFeature("Material", true),
	mData(data)
	{

	}

	virtual void load()
	{
		if (mLoaded)
			return;

		MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
		mContainer = layMan->load("ChooseListItem.layout");

		MyGUI::Widget* w = mContainer.front();

		if (!w)
			throw std::runtime_error("ItemList not found");

		mList = w->castType<MyGUI::List>(true);

		mList->eventListSelectAccept = MyGUI::newDelegate(this, &MaterialChange::onMatSelected);

		mLoaded = true;
		deactivate();
	}

	virtual void unload()
	{
		if (!mLoaded)
			return;

		if (mActive)
			deactivate();

		MyGUI::LayoutManager* layMan = MyGUI::LayoutManager::getInstancePtr();
		layMan->unloadLayout(mContainer);

		mLoaded = false;
	}

	virtual void activate()
	{
		if (mData->mActiveMesh == NULL_HANDLE)
			return;

		if (!mLoaded)
			load();

		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(true);
		}

		Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();

		Ogre::MaterialManager::ResourceMapIterator resIt = matMan->getResourceIterator();

		while(resIt.hasMoreElements())
		{
			Ogre::MaterialPtr mat = matMan->getByHandle(resIt.peekNextKey());
			mList->addItem(mat->getName());
			resIt.getNext();
		}

		mActive = true;
	}
	virtual void deactivate()
	{
		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(false);
		}

		mList->removeAllItems();

		mActive = false;
	}

	virtual void eventHandler(BFG::Controller_::VipEvent* ve)
	{

	}

private:

	void onMatSelected(MyGUI::List* list, size_t index)
	{
		const std::string& matName = list->getItemNameAt(index);

		Ogre::SceneManager* sceneMgr = Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

		if (mData->mSelectedSubEntity)
		{
			mData->mSelectedSubEntity->setMaterialName(matName);
		}
		else if (sceneMgr->hasEntity(stringify(mData->mActiveMesh)))
		{
			Ogre::Entity* ent = sceneMgr->getEntity(stringify(mData->mActiveMesh));

			ent->setMaterialName(matName);
		}

		mData->mMaterialName = matName;
	}

	boost::shared_ptr<SharedData> mData;

	MyGUI::List* mList;

}; // class AdapterControl

} // namespace Tool
#endif