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

#include <View/ControllerMyGuiAdapter.h>

#include <boost/foreach.hpp>
#include <MyGUI.h>
#include <OISMouse.h>
#include <EventSystem/Core/EventLoop.h>
#include <Controller/ControllerEvents.h>
#include <Controller/OISUtils.h>

namespace BFG {
namespace View {

using BFG::f32;

ControllerMyGuiAdapter::ControllerMyGuiAdapter(BFG::GameHandle stateHandle,
                                               EventLoop* loop) :
mAdapterLoop(loop)
{
	mAdapterEvents.push_back(BFG::ID::A_MOUSE_MOVE_X);
	mAdapterEvents.push_back(BFG::ID::A_MOUSE_MOVE_Y);
	mAdapterEvents.push_back(BFG::ID::A_MOUSE_MOVE_Z);
	mAdapterEvents.push_back(BFG::ID::A_MOUSE_LEFT_PRESSED);
	mAdapterEvents.push_back(BFG::ID::A_MOUSE_RIGHT_PRESSED);
	mAdapterEvents.push_back(BFG::ID::A_KEY_PRESSED);
	mAdapterEvents.push_back(BFG::ID::A_KEY_RELEASED);

	BOOST_FOREACH(BFG::ID::ControllerAction ca, mAdapterEvents)
	{
		loop->connect(ca, this, &ControllerMyGuiAdapter::eventHandler);
	}
}

ControllerMyGuiAdapter::~ControllerMyGuiAdapter()
{
	BOOST_FOREACH(BFG::ID::ControllerAction ca, mAdapterEvents)
	{
		mAdapterLoop->disconnect(ca, this);
	}	
}

void ControllerMyGuiAdapter::eventHandler(BFG::Controller_::VipEvent* event)
{
	switch(event->getId())
	{
	case BFG::ID::A_MOUSE_MOVE_X:
	{
		mMouseBuffer.x = boost::get<f32>(event->getData());
		MyGUI::Gui::getInstance().injectMouseMove
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			mMouseBuffer.z
		);
		break;
	}
	
	case BFG::ID::A_MOUSE_MOVE_Y:
	{
		mMouseBuffer.y = boost::get<f32>(event->getData());
		MyGUI::Gui::getInstance().injectMouseMove
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			mMouseBuffer.z
		);
		break;
	}
	
	case BFG::ID::A_MOUSE_MOVE_Z:
	{
		mMouseBuffer.z = boost::get<f32>(event->getData());
		MyGUI::Gui::getInstance().injectMouseMove
		(
			mMouseBuffer.x,
			mMouseBuffer.y,
			mMouseBuffer.z
		);
		break;
	}
	
	case BFG::ID::A_MOUSE_LEFT_PRESSED:
	{
		if (boost::get<bool>(event->getData()))
		{
			MyGUI::Gui::getInstance().injectMousePress
			(
				mMouseBuffer.x,
				mMouseBuffer.y,
				MyGUI::MouseButton::Enum(OIS::MB_Left)
			);
		}
		else
		{
			MyGUI::Gui::getInstance().injectMouseRelease
			(
				mMouseBuffer.x,
				mMouseBuffer.y,
				MyGUI::MouseButton::Enum(OIS::MB_Left)
			);
		}
		break;
	}
	
	case BFG::ID::A_MOUSE_RIGHT_PRESSED:
	{
		if (boost::get<bool>(event->getData()))
		{
			MyGUI::Gui::getInstance().injectMousePress
			(
				mMouseBuffer.x,
				mMouseBuffer.y,
				MyGUI::MouseButton::Enum(OIS::MB_Right)
			);
		}
		else
		{
			MyGUI::Gui::getInstance().injectMouseRelease
			(
				mMouseBuffer.x,
				mMouseBuffer.y,
				MyGUI::MouseButton::Enum(OIS::MB_Right)
			);
		}
		break;
	}

	case BFG::ID::A_KEY_PRESSED:
	case BFG::ID::A_KEY_RELEASED:
	{
		BFG::ID::KeyboardButton code = (BFG::ID::KeyboardButton) boost::get<BFG::s32>(event->getData());

		OIS::KeyCode ois_key;
		BFG::s32 ois_ch;

		BFG::Controller_::Utils::convertToBrainDamagedOisStyle(code, ois_key, ois_ch);
		MyGUI::KeyCode::Enum mygui_key = (MyGUI::KeyCode::Enum) ois_key;
		MyGUI::Char mygui_ch = ois_ch;
		
		if (event->getId() == BFG::ID::A_KEY_PRESSED)
		{
			MyGUI::Gui::getInstance().injectKeyPress(mygui_key, mygui_ch);
		}
		else // A_KEY_RELEASED
		{
			if (mygui_key == MyGUI::KeyCode::None)
				mygui_key = MyGUI::KeyCode::Enum(mygui_ch);

			MyGUI::Gui::getInstance().injectKeyRelease(mygui_key);
		}

		break;
	}
	}
}

} // namespace View
} // namespace BFG