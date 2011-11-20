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

#ifndef BFG_VIEWMAIN_H
#define BFG_VIEWMAIN_H

#ifdef _MSC_VER
  #pragma warning (push)
  // "class foo needs to have dll-interface to be used by clients of class bar"
  #pragma warning (disable:4251)
#endif

#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/Types.h>
#include <EventSystem/Core/EventLoop.h>

#include <View/Event_fwd.h>
#include <View/Interface.h>

namespace Ogre
{
	class Root;
	class SceneManager;
}

namespace MyGUI
{
	class Gui;
	class OgrePlatform;
}

namespace BFG {
namespace View {

class State;
class Camera;
class Fps;
class Console;
struct StateInsertion;

class VIEW_API Main
{
public:
	Main(EventLoop* loop, const std::string& windowTitle);
	virtual ~Main();

	static EventLoop* eventLoop();

private:
	// Ogre		
	void initOgre();
	void initializeRoot();
	void loadResources();
	void createSceneManager();
	void createMainCamera();

	// MyGui
	void initMyGui();

	void eventHandler(Event* VE);
	
	void onScreenShot();
	void onDebugFps(bool enable);
	void onConsole(bool enable);
	
	void loopEventHandler(LoopEvent* iLE);

	bool doRenderTick();

	static EventLoop* mLoop;

	bool              mShutdown;
	const std::string mWindowTitle;
	
	// Leave this on top since the auto destruction order is bottom to top,
	// and mRoot must be deleted last.
	boost::shared_ptr<Ogre::Root> mRoot;
	Ogre::SceneManager* mSceneMgr;

	// for initialization only !!
	boost::scoped_ptr<Camera> mMainCamera;
	boost::scoped_ptr<Fps> mFps;
	boost::scoped_ptr<Console> mConsole;
	
	// Leave this MyGui-pointer order because gui must be deleted first.
	boost::shared_ptr<MyGUI::OgrePlatform> mPlatform;
	boost::shared_ptr<MyGUI::Gui> mGui;
};

} // namespace View
} // namespace BFG

#ifdef _MSC_VER
  #pragma warning (pop)
#endif

#endif

