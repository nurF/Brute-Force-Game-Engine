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

#include <View/MassTest.h>

#include <OgreRoot.h>
#include <OgreSceneManager.h>

#include <Core/Utils.h> // generateHandle()

#include <View/RenderObject.h>
#include <View/Defs.h>


namespace BFG {
namespace View {


MassTest::MassTest() :
mLight(NULL),
arc(0.0f)
{
	using namespace Ogre;
	SceneManager* sceneMgr = Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	mLight = sceneMgr->createLight("bla oder blub");

	mLight->setType(Light::LT_DIRECTIONAL);
	mLight->setDirection(Vector3(1.0f, 1.0f, 1.0f));
	mLight->setDiffuseColour(ColourValue::White);
	mLight->setSpecularColour(ColourValue::White);
	mLight->setPowerScale(5.0f);

	sceneMgr->setAmbientLight(ColourValue::Black);
	
	GameHandle handle = generateHandle();

	// x
	size_t j_max = 10;
	float j_dist = 10.0f;

	// y
	size_t k_max = 20;
	float k_dist = 5.0f;
	
	// z
	size_t i_max = 10;
	float i_dist = 20.0f;
	
 	for (size_t k=0; k<k_max; ++k)
 	{	
 		for (size_t j=0; j<j_max; ++j)
 		{	
			for (size_t i=0; i<i_max; ++i)
			{	
				boost::shared_ptr<RenderObject> tmp;
				tmp.reset(new RenderObject
				(
					NULL_HANDLE,
					generateHandle(),
					"FastRacer.mesh",
					v3
					(
						j*j_dist - j_max/2*j_dist,
						k*k_dist - k_max/2*k_dist - k_dist/2,
						i_dist + i*i_dist
					),
					qv4(0,1,0,0)
				));
				mObjects.push_back(tmp);
			}
		}
	}
}

MassTest::~MassTest()
{
	using namespace Ogre;
	SceneManager* sceneMgr = Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);
	sceneMgr->destroyLight(mLight);
}

void MassTest::update(f32 timeSinceLastFrame)
{
	arc += timeSinceLastFrame;

	mLight->setDirection(cos(arc), sin(arc), 1.0f);
	
}

} // namespace View
} // namespace BFG