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

#include <View/Explosion.h>

#include <stdexcept>

#include <OgreRoot.h>
#include <OgreBillboard.h>
#include <OgreBillboardSet.h>
#include <OgreParticleSystem.h>
#include <OgreColourValue.h>

#include <Core/Utils.h> // generateHandle(), stringify()
#include <View/Defs.h>

namespace BFG {
namespace View {

void fade(Ogre::Billboard* billboard,
          f32 startTime,
          f32 endTime,
          f32 elapsedTime,
          bool fadeOut)
{
	if (elapsedTime > endTime) // <--- :TODO: call this just once instead of every time the 'if' above is false
	{
		Ogre::ColourValue cv = billboard->getColour();
		f32 fadeEnd = 1.0f;

		if (fadeOut)
			fadeEnd = 0.0f;

		billboard->setColour(Ogre::ColourValue(cv.r, cv.g, cv.b, fadeEnd));
		return;
	}
	
	if (elapsedTime > startTime)
	{
		f32 fader = (elapsedTime - startTime) / (endTime - startTime);

		if (fadeOut)
			fader = 1.0f - fader;

		Ogre::ColourValue cv = billboard->getColour();
		billboard->setColour(Ogre::ColourValue(cv.r, cv.g, cv.b, fader));
		return;
	}
}

void expand(Ogre::Billboard* billboard,
            f32 startTime,
            f32 endTime,
            f32 elapsedTime,
            f32 velocity)
{
	if (elapsedTime > startTime && elapsedTime <= endTime)
	{
		f32 time = elapsedTime - startTime;
		f32 fader = velocity * time * time;
		billboard->setDimensions(fader, fader);
	}
}

void shrink(Ogre::Billboard* billboard,
            f32 startTime,
            f32 endTime,
            f32 elapsedTime,
            f32 startSize)
{
	if (elapsedTime > endTime) // <--- :TODO: call this just once instead of every time the 'if' above is false
	{
		billboard->setDimensions(0.0f, 0.0f);
		return;
	}
	if (elapsedTime > startTime)
	{
		f32 fader = 1.0f - (elapsedTime - startTime) / (endTime - startTime);
		f32 scale = fader * startSize;
		billboard->setDimensions(scale, scale);
		return;
	}
}

void fireParticles(Ogre::ParticleSystem* particleSystem,
                   Ogre::SceneNode* sceneNode,
                   f32 startTime,
                   f32 endTime,
                   f32 elapsedTime)
{
	if ((elapsedTime > startTime) && (elapsedTime < endTime))
	{
		if(!particleSystem->isAttached())
			sceneNode->attachObject(particleSystem);
		return;
	}
	if (elapsedTime > endTime)
	{
		if (particleSystem->isAttached())
			sceneNode->detachObject(particleSystem);	
		return;
	}
}

void fireScaledParticles(Ogre::ParticleSystem* particleSystem,
						 Ogre::SceneNode* sceneNode,
						 f32 startWidth,
						 f32 startHeight,
                         f32 widthFactor,
                         f32 heightFactor,
                         f32 startTime,
                         f32 endTime,
                         f32 elapsedTime)
{
	fireParticles(particleSystem, sceneNode, startTime, endTime, elapsedTime);
	if (elapsedTime > startTime && elapsedTime < endTime)
	{
		f32 factor = (elapsedTime - startTime) / (endTime - startTime);
	
		particleSystem->setDefaultWidth(startWidth + startWidth * widthFactor * factor);
		particleSystem->setDefaultHeight(startHeight + startHeight * heightFactor * factor);
	}
}


Explosion::Explosion(const v3& position, f32 intensity) :
mPosition(position),
mIntensity(intensity),  //! \todo <- Use me! (1.0f is "normal")
mBillboardSet(NULL),
mElapsedTime(0),
mHandle(generateHandle()),
mParticleSystem(NULL)
{

	Ogre::SceneManager* sceneMgr =
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	Ogre::SceneNode* myNode = 
		static_cast<Ogre::SceneNode*>(sceneMgr->getRootSceneNode()->createChild());
		
	if (!myNode)
		throw std::runtime_error("Explosion: creating myNode failed!");

	myNode->setPosition(Ogre::Vector3(mPosition.ptr()));

	mBillboardSet = sceneMgr->createBillboardSet(stringify(mHandle));
	if (!mBillboardSet)
		throw std::runtime_error("Explosion: creating mBillboardSet failed!");
	
	v3 bias = v3(0, 0, 0.01f);

	Ogre::Billboard* b1 = mBillboardSet->createBillboard
	(
		Ogre::Vector3(mPosition.ptr()),
        Ogre::ColourValue(1.0f, 0.9f, 0.0f)
	);
		
	if (!b1)
		throw std::runtime_error("Explosion: creating b1 failed!");
		                                                     
	b1->setDimensions(10, 10);

	Ogre::Billboard* b2 = mBillboardSet->createBillboard
	(
		Ogre::Vector3((mPosition - bias).ptr()),
		Ogre::ColourValue(1.0f, 1.0f, 0.66f)
	);
		
	if (!b2)
		throw std::runtime_error("Explosion: creating b2 failed!");

	b2->setDimensions(5, 5);

	Ogre::Billboard* b3 = mBillboardSet->createBillboard
	(
		Ogre::Vector3((mPosition - bias*3).ptr()),
		Ogre::ColourValue(1.0f, 1.0f, 0.0f, 0.0f)
	);
		
	if (!b3)
		throw std::runtime_error("Explosion: creating b3 failed!");

	b3->setDimensions(60, 60);

	Ogre::Billboard* b4 = mBillboardSet->createBillboard
	(
		Ogre::Vector3((mPosition - bias*2).ptr()),
		Ogre::ColourValue(1.0f, 0.66f, 0.0f, 0.0f)
	);
		
	if (!b4)
		throw std::runtime_error("Explosion: creating b4 failed!");

	b4->setDimensions(60, 60);

	mBillboardSet->setMaterialName("Explosion_blob");

	myNode->attachObject(mBillboardSet);

	mParticleSystem = sceneMgr->createParticleSystem
	(
		stringify(mHandle) + "particle",
		"Explosion"
	);
	
	if (!mParticleSystem)
		throw std::runtime_error("Explosion: creating mParticleSystem failed!");

}

Explosion::~Explosion()
{
	Ogre::SceneManager* sceneMgr =
		Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	Ogre::SceneNode* myNode = mBillboardSet->getParentSceneNode();

	sceneMgr->destroySceneNode(myNode);

	for (; 0 < mBillboardSet->getNumBillboards();)
	{
		mBillboardSet->removeBillboard((unsigned int)0);
	}

	sceneMgr->destroyParticleSystem(mParticleSystem);

	sceneMgr->destroyBillboardSet(mBillboardSet);
}

bool Explosion::frameStarted( const Ogre::FrameEvent& evt )
{
	if (done())
		return true;

	mElapsedTime += evt.timeSinceLastFrame;

	expand(mBillboardSet->getBillboard(0), 0.0f, 0.5f, mElapsedTime, 1000.0f);
	expand(mBillboardSet->getBillboard(1), 0.0f, 0.5f, mElapsedTime, 7500.0f);

	fade(mBillboardSet->getBillboard(0), 0.5f, 9.0f, mElapsedTime, true);
	fade(mBillboardSet->getBillboard(1), 0.5f, 9.0f, mElapsedTime, true);

	fade(mBillboardSet->getBillboard(2), 3.0f, 3.5f, mElapsedTime, false);
	fade(mBillboardSet->getBillboard(3), 3.5f, 4.0f, mElapsedTime, false);

 	fireParticles
 	(
 		mParticleSystem,
 		mBillboardSet->getParentSceneNode(),
 		2.0f,
 		9.0f,
 		mElapsedTime
 	);

	shrink(mBillboardSet->getBillboard(2), 3.5f, 6.0f, mElapsedTime, 60.0f);
	fade(mBillboardSet->getBillboard(2), 3.5f, 6.0f, mElapsedTime, true);

	shrink(mBillboardSet->getBillboard(3), 4.0f, 6.5f, mElapsedTime, 60.0f);
	fade(mBillboardSet->getBillboard(3), 4.0f, 6.5f, mElapsedTime, true);
	
	if (mElapsedTime > 9.0f)
		finished();
	
	return true;
}


// ##############
// # Explosion2 #
// ##############


Explosion2::Explosion2(const v3& position, f32 intensity) :
mPosition(position),
mIntensity(intensity),  //! \todo <- Use me! (1.0f is "normal")
mElapsedTime(0),
mHandle(generateHandle())
{
	Ogre::SceneManager* sceneMgr = 
		Ogre::Root::getSingleton().getSceneManager(BFG_SCENEMANAGER);

	mNode = static_cast<Ogre::SceneNode*>
		(sceneMgr->getRootSceneNode()->createChild(stringify(mHandle)));
	
	mNode->setPosition(Ogre::Vector3(mPosition.ptr()));

	mParticles["Fire"] = sceneMgr->createParticleSystem
    (
		stringify(mHandle) + "fire",
		"Explosion_01_fire"
	);

	mParticles["Flash"] = sceneMgr->createParticleSystem
	(
		stringify(mHandle) + "flash",
		"Explosion_01_flash"
	);

	mParticles["Sparcles"] = sceneMgr->createParticleSystem
    (
		stringify(mHandle) + "sparcles",
		"Explosion_01_sparcles"
	);

	mParticles["Smoketrail"] = sceneMgr->createParticleSystem
	(
		stringify(mHandle) + "smoketrail",
		"Explosion_01_smoketrail"
	);
	
	mParticles["Shockwave"] = sceneMgr->createParticleSystem
	(
		stringify(mHandle) + "shockwave",
		"Explosion_01_shockwave"
	);

	mStartWidth  = mParticles["Shockwave"]->getDefaultWidth();
	mStartHeight = mParticles["Shockwave"]->getDefaultHeight();
}

Explosion2::~Explosion2()
{
	Ogre::SceneManager* sceneMgr =
		Ogre::Root::getSingletonPtr()->getSceneManager(BFG_SCENEMANAGER);

	if(!sceneMgr)
		return;

	if (mNode)
		sceneMgr->destroySceneNode(mNode);

	ParticleMapT::iterator it = mParticles.begin();
	for(; it != mParticles.end(); ++it)
	{
		sceneMgr->destroyParticleSystem(it->second);
	}
}

bool Explosion2::frameStarted(const Ogre::FrameEvent& evt)
{
	if (done())
		return true;

	mElapsedTime += evt.timeSinceLastFrame;

	fireParticles(mParticles["Flash"], mNode, 0.0f, 1.5f, mElapsedTime);
	fireParticles(mParticles["Fire"], mNode, 0.0f, 1.5f, mElapsedTime);
	fireParticles(mParticles["Sparcles"], mNode, 0.0f, 1.5f, mElapsedTime);
	fireParticles(mParticles["Smoketrail"], mNode, 0.0f, 1.5f, mElapsedTime);

	fireScaledParticles
	(
		mParticles["Shockwave"],
		mNode,
		mStartWidth,
		mStartHeight,
		4.0f,
		2.0f,
		0.0f,
		1.5f,
		mElapsedTime
	);

	if (mElapsedTime > 4.0f)
		finished();

	return true;
}


} // namespace View
} // namespace BFG
