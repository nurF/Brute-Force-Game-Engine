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

#include <View/Light.h>

#include <stdexcept>

#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <Core/Utils.h>

#include <View/Convert.h>
#include <View/Defs.h>
#include <View/LightCreation.h>

namespace BFG {
namespace View {

Light::Light(const LightCreation& LC) :
mHandle(LC.mHandle)
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr = root.getSceneManager(BFG_SCENEMANAGER);

	Ogre::Light* light = sceneMgr->createLight(stringify(LC.mHandle));

	light->setType(static_cast<Ogre::Light::LightTypes>(LC.mType));
	light->setDiffuseColour(LC.mDiffuse);
	light->setSpecularColour(LC.mSpecular);
	light->setPowerScale(LC.mPower);
	
	switch(LC.mType)
	{
		case ID::LT_Point:
			createPointLight(LC, light);
		break;

		case ID::LT_Directional:
			createDirectionalLight(LC, light);
		break;

		case ID::LT_Spot:
			createSpotLight(LC, light);
		break;

		default:
		throw std::logic_error("RaceState: onCreateLight: unknown LightType!");
	}
}

Light::~Light()
{
	Ogre::Root& root = Ogre::Root::getSingleton();
	Ogre::SceneManager* sceneMgr = root.getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->destroyLight(stringify(mHandle));
}

void Light::createSpotLight(const LightCreation& LC, Ogre::Light*& light)
{
	const SpotLightCreation& slc = 
		static_cast<const SpotLightCreation&>(LC);
	light->setPosition(toOgre(slc.mPosition));
	light->setDirection(toOgre(slc.mDirection));
	light->setAttenuation
	(
		slc.mRange,
		slc.mConstant,
		slc.mLinear,
		slc.mQuadric
	);
	light->setSpotlightRange
	(
		Ogre::Radian(slc.mInnerRadius),
		Ogre::Radian(slc.mOuterRadius),
		slc.mFalloff
	);
}

void Light::createDirectionalLight(const LightCreation& LC, Ogre::Light*& light)
{
	const DirectionalLightCreation& dlc = 
		static_cast<const DirectionalLightCreation&>(LC);
	light->setDirection(toOgre(dlc.mDirection));
}

void Light::createPointLight(const LightCreation& LC, Ogre::Light*& light)
{
	const PointLightCreation& plc = 
		static_cast<const PointLightCreation&>(LC);
	light->setPosition(toOgre(plc.mPosition));
	light->setAttenuation
	(
		plc.mRange,
		plc.mConstant,
		plc.mLinear,
		plc.mQuadric
	);
}

} // namespace View
} // namespace BFG
