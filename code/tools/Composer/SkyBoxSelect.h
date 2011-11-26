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

#ifndef SKYBOXSELECT
#define SKYBOXSELECT

#include <BaseFeature.h>

#include <SharedData.h>

namespace Tool
{

class SkyBoxSelect : public BaseFeature, public Emitter
{
public:
	SkyBoxSelect(EventLoop* loop, boost::shared_ptr<SharedData> data) :
	BaseFeature("SkyBox", true),
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

		mList->eventListSelectAccept = MyGUI::newDelegate(this, &SkyBoxSelect::onSkySelected);

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
		MyGUI::VectorWidgetPtr::iterator it = mContainer.begin();
		for (; it != mContainer.end(); ++it)
		{
			(*it)->setVisible(true);
		}

		Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();

		Ogre::MaterialManager::ResourceMapIterator resIt = matMan->getResourceIterator();

		std::string matName;

		while(resIt.hasMoreElements())
		{
			matName = "";
			Ogre::MaterialPtr mat = matMan->getByHandle(resIt.peekNextKey());
			for (size_t index = 0; index < mat->getNumTechniques(); ++index)
			{
				Ogre::Technique* tech = mat->getTechnique(index);
				for (size_t passIndex = 0; passIndex < tech->getNumPasses(); ++passIndex)
				{
					Ogre::Pass* pass = tech->getPass(passIndex);
					for (size_t texIndex = 0; texIndex < pass->getNumTextureUnitStates(); ++texIndex)
					{
						Ogre::TextureUnitState* tex = pass->getTextureUnitState(texIndex);
						if(tex->isCubic())
							matName = mat->getName();
					}
				}
			}
			if ( (matName != "") && (matName.substr(0, 16) != BFG_SCENEMANAGER) )
			{
				mList->addItem(mat->getName());
			}
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

	void onSkySelected(MyGUI::List* list, size_t index)
	{
		const std::string& skyName = list->getItemNameAt(index);

		View::SkyCreation sc(skyName);
		emit<View::Event>(ID::VE_SET_SKY, sc, mData->mState);

		Ogre::TextureUnitState* skyTexUnit = findAliasInMaterial("sky", skyName);

		if (!skyTexUnit)
			return;

		std::vector<Ogre::TextureUnitState*> texVec = findTextureAlias("Environment");

		std::vector<Ogre::TextureUnitState*>::iterator it = texVec.begin();
		for (; it != texVec.end(); ++it)
		{
			Ogre::TextureUnitState* tex = *it;

			std::string texName = skyTexUnit->getTextureName();
			std::string extension = texName.substr(texName.find_last_of("."));
			std::string name = texName.substr(0, texName.find("_fr"));

			tex->setCubicTextureName(name + extension, true);
			tex->setColourOperationEx(Ogre::LBX_ADD,
			                          Ogre::LBS_TEXTURE,
			                          Ogre::LBS_CURRENT);
			tex->setColourOpMultipassFallback(Ogre::SBF_ONE,
			                                  Ogre::SBF_ONE);
			tex->setEnvironmentMap(true, Ogre::TextureUnitState::ENV_REFLECTION);
		}

		deactivate();
	}

	std::vector<Ogre::TextureUnitState*> findTextureAlias(const std::string alias)
	{
		Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();
		Ogre::MaterialManager::ResourceMapIterator::iterator resIt =
			matMan->getResourceIterator().begin();

		std::vector<Ogre::TextureUnitState*> result;
		for (; resIt != matMan->getResourceIterator().end(); ++resIt)
		{
			Ogre::MaterialPtr mat = resIt->second;

			Ogre::Material::TechniqueIterator techIt = mat->getTechniqueIterator();
			while(techIt.hasMoreElements())
			{
				Ogre::Technique* tech = techIt.getNext();

				Ogre::Technique::PassIterator passIt = tech->getPassIterator();
				while(passIt.hasMoreElements())
				{
					Ogre::Pass* pass = passIt.getNext();

					Ogre::Pass::TextureUnitStateIterator texIt = pass->getTextureUnitStateIterator();

					while (texIt.hasMoreElements())
					{
						Ogre::TextureUnitState* tex = texIt.getNext();

						if (tex->getTextureNameAlias() == alias)
						{
							result.push_back(tex);
						}
					}
				}
			}
		}
		return result;
	}

	Ogre::TextureUnitState* findAliasInMaterial(const std::string alias, const std::string matName)
	{
		Ogre::MaterialManager* matMan = Ogre::MaterialManager::getSingletonPtr();
		Ogre::MaterialPtr mat = matMan->getByName(matName);

		Ogre::Material::TechniqueIterator techIt = mat->getTechniqueIterator();
		while(techIt.hasMoreElements())
		{
			Ogre::Technique* tech = techIt.getNext();

			Ogre::Technique::PassIterator passIt = tech->getPassIterator();
			while(passIt.hasMoreElements())
			{
				Ogre::Pass* pass = passIt.getNext();

				Ogre::Pass::TextureUnitStateIterator texIt = pass->getTextureUnitStateIterator();

				while (texIt.hasMoreElements())
				{
					Ogre::TextureUnitState* tex = texIt.getNext();

					if (tex->getTextureNameAlias() == alias)
					{
						return tex;
					}
				}
			}
		}
		return NULL;
	}

	boost::shared_ptr<SharedData> mData;

	MyGUI::List* mList;

}; // class SkyBoxSelect

} // namespace Tool
#endif