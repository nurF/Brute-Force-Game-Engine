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

#ifndef SUBENTITYSELECT
#define SUBENTITYSELECT

#include <BaseFeature.h>

#include <Event_fwd.h>
#include <SharedData.h>

namespace Tool
{

class SubEntitySelect : public BaseFeature, public Emitter
{
public:
	SubEntitySelect(EventLoop* loop,boost::shared_ptr<SharedData> data) :
	BaseFeature("SubEntity", true),
	Emitter(loop),
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

		mList->eventListSelectAccept = MyGUI::newDelegate(this, &SubEntitySelect::onEntitySelected);

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

		mMeshName = mData->mMeshName;

		if (!mLoaded)
			load();

		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(true);
		}

		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		if (sceneMgr->hasEntity(stringify(mData->mActiveMesh)))
		{
			Ogre::Entity* ent = sceneMgr->getEntity(stringify(mData->mActiveMesh));

			Ogre::MeshPtr mesh = ent->getMesh();

			const Ogre::Mesh::SubMeshNameMap subMap = mesh->getSubMeshNameMap();

			Ogre::Mesh::SubMeshNameMap::const_iterator subIt = subMap.begin();
			for (; subIt != subMap.end(); ++subIt)
			{
				mList->addItem(subIt->first);
			}
		}

		mActive = true;

		emit<Tool::Event>(A_UPDATE_FEATURES, 0);
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

		emit<Tool::Event>(A_UPDATE_FEATURES, 0, mData->mState);
	}

	virtual void eventHandler(BFG::Controller_::VipEvent* ve)
	{

	}

	virtual void update(const Ogre::FrameEvent& evt)
	{
		if (mMeshName != mData->mMeshName)
		{
			deactivate();
			activate();
		}
	}

private:

	void onEntitySelected(MyGUI::List* list, size_t index)
	{
		std::string subName = list->getItemNameAt(index);

		Ogre::SceneManager* sceneMgr = 
			Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

		if (sceneMgr->hasEntity(stringify(mData->mActiveMesh)))
		{
			Ogre::Entity* ent = sceneMgr->getEntity(stringify(mData->mActiveMesh));

			mData->mSelectedSubEntity = ent->getSubEntity(subName);
		}

		deactivate();
	}
	boost::shared_ptr<SharedData> mData;

	MyGUI::List* mList;
	std::string mMeshName;

}; // class SubEntitySelect

} // namespace Tool
#endif