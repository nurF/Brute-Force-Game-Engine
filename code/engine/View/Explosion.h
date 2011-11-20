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

#ifndef EXPLOSION_H_
#define EXPLOSION_H_

#include <boost/noncopyable.hpp>

#include <Core/Types.h>
#include <Core/v3.h>

#include <View/Effect.h>
#include <View/Defs.h>

namespace Ogre
{
	class BillboardSet;
	class ParticleSystem;
}

namespace BFG {
namespace View {


class VIEW_API Explosion : boost::noncopyable, public Effect
{
public:
	//! \param intensity
	//!		This parameter is not yet implemented.
	Explosion(const v3& position, f32 intensity);
	~Explosion();

	bool frameStarted(const Ogre::FrameEvent& evt);

private:

	v3                  mPosition;
	f32                 mIntensity;
	Ogre::BillboardSet* mBillboardSet;
	f32                 mElapsedTime;
	GameHandle          mHandle;
	Ogre::ParticleSystem* mParticleSystem;
};


class VIEW_API Explosion2 : boost::noncopyable, public Effect
{
public:
	//! \param intensity
	//!		This parameter is not yet implemented.
	Explosion2(const v3& position, f32 intensity);
	~Explosion2();

	bool frameStarted(const Ogre::FrameEvent& evt);

private:
	typedef std::map<std::string, Ogre::ParticleSystem*> ParticleMapT;

	ParticleMapT mParticles;

	f32	mStartWidth;
	f32 mStartHeight;

	Ogre::SceneNode* mNode;

	v3 mPosition;
	f32 mIntensity;

	f32 mElapsedTime;
	GameHandle mHandle;
};


} // namespace View
} // namespace BFG

#endif