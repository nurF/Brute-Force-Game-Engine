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

#include <View/Skybox.h>

#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <View/Defs.h>


namespace BFG {
namespace View {

bool Skybox::mOldSkyBoxDestroyed = true;

Skybox::Skybox(const std::string& skyMat)
{
	assert(mOldSkyBoxDestroyed &&
		"A skybox has to be destroyed before it can be recreated. scoped- and "
		"shared-ptr call constructor first and destructor second when reset is "
		"called. Therefore you need to call a simple reset() first.");
	
	Ogre::SceneManager* sceneMgr = 
		Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->setSkyBox(true, skyMat, 80000);
	mOldSkyBoxDestroyed = false;
}

Skybox::~Skybox()
{
	Ogre::SceneManager* sceneMgr = 
		Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	sceneMgr->setSkyBox(false, "");
	mOldSkyBoxDestroyed = true;
}


} // namespace View
} // namespace BFG
