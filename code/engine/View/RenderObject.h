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
#ifndef __RENDER_OBJECT__
#define __RENDER_OBJECT__

#include <View/Event_fwd.h>

namespace BFG {
namespace View {

class VIEW_API RenderObject
{
public:
	RenderObject(GameHandle parent,
	             GameHandle handle,
	             const std::string& meshName,
	             const v3& position = v3::ZERO,
	             const qv4& orientation = qv4::IDENTITY);

	~RenderObject();

private:
	void viewEventHandler(Event* VE);

	void updatePosition(const v3& position);
	void updateOrientation(const qv4& orientation);

	void onSetVisible(bool visibility);

	// This is only to attach "graphical objects" directly to the own scene Node.
	// The relation is: Entity n -> 1 SceneNode.
	void attachOgreObject(Ogre::MovableObject* aObject);
	void detachEntity(const std::string& name);

	void onAttachObject(GameHandle child);
	void onDetachObject();
	void removeChildNode(const std::string& name);

	// To identify object in the render-engine.
	GameHandle mHandle;

	Ogre::SceneNode* mSceneNode;
	Ogre::Entity* mEntity;

};
} // namespace View
} // namespace BFG

#endif
