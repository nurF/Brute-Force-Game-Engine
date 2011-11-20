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
#include <Controller/OISUtils.h>

#include <sstream>
#include <OISInputManager.h>
#include <Base/CLogger.h>

namespace BFG {
namespace Controller_ {
namespace Utils {

//! This is the exact contrary of:
//! bool translateKeyBack(ID::KeyboardButton code, OIS::KeyCode& result)
//! If you modify this function, you need to modify the other one too.
bool translateKey(OIS::KeyCode code, ID::KeyboardButton& result)
{
	if (code >= OIS::KC_F1 &&
	    code <= OIS::KC_F10)
	{
		result = static_cast<ID::KeyboardButton>(code + 0xDF);
		return true;
	}

	if (code >= OIS::KC_F11 &&
	    code <= OIS::KC_F15)
	{
		result = static_cast<ID::KeyboardButton>(code + 0xCD);
		return true;
	}

	switch (code)
	{
		case OIS::KC_GRAVE: result = ID::KB_CARET; break;
		
		case OIS::KC_NUMPAD0: result = ID::KB_KP0; break;
		case OIS::KC_NUMPAD1: result = ID::KB_KP1; break;
		case OIS::KC_NUMPAD2: result = ID::KB_KP2; break;
		case OIS::KC_NUMPAD3: result = ID::KB_KP3; break;
		case OIS::KC_NUMPAD4: result = ID::KB_KP4; break;
		case OIS::KC_NUMPAD5: result = ID::KB_KP5; break;
		case OIS::KC_NUMPAD6: result = ID::KB_KP6; break;
		case OIS::KC_NUMPAD7: result = ID::KB_KP7; break;
		case OIS::KC_NUMPAD8: result = ID::KB_KP8; break;
		case OIS::KC_NUMPAD9: result = ID::KB_KP9; break;

		case OIS::KC_DECIMAL:      result = ID::KB_KP_PERIOD; break;
		case OIS::KC_DIVIDE:       result = ID::KB_KP_DIVIDE; break;
		case OIS::KC_MULTIPLY:     result = ID::KB_KP_MULTIPLY; break;
		case OIS::KC_SUBTRACT:     result = ID::KB_KP_MINUS; break;
		case OIS::KC_ADD:          result = ID::KB_KP_PLUS; break;
		case OIS::KC_NUMPADENTER:  result = ID::KB_KP_ENTER; break;
		case OIS::KC_NUMPADEQUALS: result = ID::KB_KP_EQUALS; break;

		case OIS::KC_UP:     result = ID::KB_UP; break;
		case OIS::KC_DOWN:   result = ID::KB_DOWN; break;
		case OIS::KC_RIGHT:  result = ID::KB_RIGHT; break;
		case OIS::KC_LEFT:   result = ID::KB_LEFT; break;
		case OIS::KC_INSERT: result = ID::KB_INSERT; break;
		case OIS::KC_HOME:   result = ID::KB_HOME; break;
		case OIS::KC_END:    result = ID::KB_END; break;
		case OIS::KC_PGUP:   result = ID::KB_PAGEUP; break;
		case OIS::KC_PGDOWN: result = ID::KB_PAGEDOWN; break;
		
		case OIS::KC_DELETE: result = ID::KB_DELETE; break;
		case OIS::KC_BACK:   result = ID::KB_BACKSPACE; break;
		case OIS::KC_RETURN: result = ID::KB_RETURN; break;

		case OIS::KC_NUMLOCK:  result = ID::KB_NUMLOCK; break;
		case OIS::KC_CAPITAL:  result = ID::KB_CAPSLOCK; break;
		case OIS::KC_SCROLL:   result = ID::KB_SCROLLOCK; break;
		case OIS::KC_RSHIFT:   result = ID::KB_RSHIFT; break;
		case OIS::KC_LSHIFT:   result = ID::KB_LSHIFT; break;
		case OIS::KC_RCONTROL: result = ID::KB_RCTRL; break;
		case OIS::KC_LCONTROL: result = ID::KB_LCTRL; break;
		case OIS::KC_RMENU:    result = ID::KB_RALT; break;
		case OIS::KC_LMENU:    result = ID::KB_LALT; break;
		case OIS::KC_RWIN:     result = ID::KB_RMETA; break;
		case OIS::KC_LWIN:     result = ID::KB_LMETA; break;
		
		default:
			return false;
	};

	return true;
}

//! This is the exact contrary of:
//! bool translateKey(OIS::KeyCode code, ID::KeyboardButton& result)
//! If you modify this function, you need to modify the other one too.
bool translateKeyBack(ID::KeyboardButton code, OIS::KeyCode& result)
{
	if (code >= ID::KB_F1 &&
	    code <= ID::KB_F10)
	{
		result = static_cast<OIS::KeyCode>(code - 0xDF);
		return true;
	}

	if (code >= ID::KB_F11 &&
	    code <= ID::KB_F15)
	{
		result = static_cast<OIS::KeyCode>(code - 0xCD);
		return true;
	}

	switch (code)
	{
		case ID::KB_CARET: result = OIS::KC_GRAVE; break;
		
		case ID::KB_KP0: result = OIS::KC_NUMPAD0; break;
		case ID::KB_KP1: result = OIS::KC_NUMPAD1; break;
		case ID::KB_KP2: result = OIS::KC_NUMPAD2; break;
		case ID::KB_KP3: result = OIS::KC_NUMPAD3; break;
		case ID::KB_KP4: result = OIS::KC_NUMPAD4; break;
		case ID::KB_KP5: result = OIS::KC_NUMPAD5; break;
		case ID::KB_KP6: result = OIS::KC_NUMPAD6; break;
		case ID::KB_KP7: result = OIS::KC_NUMPAD7; break;
		case ID::KB_KP8: result = OIS::KC_NUMPAD8; break;
		case ID::KB_KP9: result = OIS::KC_NUMPAD9; break;

		case ID::KB_KP_PERIOD: result = OIS::KC_DECIMAL; break;
		case ID::KB_KP_DIVIDE: result = OIS::KC_DIVIDE; break;
		case ID::KB_KP_MULTIPLY: result = OIS::KC_MULTIPLY; break;
		case ID::KB_KP_MINUS: result = OIS::KC_SUBTRACT; break;
		case ID::KB_KP_PLUS: result = OIS::KC_ADD; break;
		case ID::KB_KP_ENTER: result = OIS::KC_NUMPADENTER; break;
		case ID::KB_KP_EQUALS: result = OIS::KC_NUMPADEQUALS; break;

		case ID::KB_UP: result = OIS::KC_UP; break;
		case ID::KB_DOWN: result = OIS::KC_DOWN; break;
		case ID::KB_RIGHT: result = OIS::KC_RIGHT; break;
		case ID::KB_LEFT: result = OIS::KC_LEFT; break;
		case ID::KB_INSERT: result = OIS::KC_INSERT; break;
		case ID::KB_HOME: result = OIS::KC_HOME; break;
		case ID::KB_END: result = OIS::KC_END; break;
		case ID::KB_PAGEUP: result = OIS::KC_PGUP; break;
		case ID::KB_PAGEDOWN: result = OIS::KC_PGDOWN; break;

		case ID::KB_DELETE: result = OIS::KC_DELETE; break;
		case ID::KB_BACKSPACE: result = OIS::KC_BACK; break;
		case ID::KB_RETURN: result = OIS::KC_RETURN; break;

		case ID::KB_NUMLOCK: result = OIS::KC_NUMLOCK; break;
		case ID::KB_CAPSLOCK: result = OIS::KC_CAPITAL; break;
		case ID::KB_SCROLLOCK: result = OIS::KC_SCROLL; break;
		case ID::KB_RSHIFT: result = OIS::KC_RSHIFT; break;
		case ID::KB_LSHIFT: result = OIS::KC_LSHIFT; break;
		case ID::KB_RCTRL: result = OIS::KC_RCONTROL; break;
		case ID::KB_LCTRL: result = OIS::KC_LCONTROL; break;
		case ID::KB_RALT: result = OIS::KC_RMENU; break;
		case ID::KB_LALT: result = OIS::KC_LMENU; break;
		case ID::KB_RMETA: result = OIS::KC_RWIN; break;
		case ID::KB_LMETA: result = OIS::KC_LWIN; break;
		
		default:
			return false;
	};

	return true;
}

void convertToBrainDamagedOisStyle(ID::KeyboardButton code,
                                   OIS::KeyCode& key,
                                   s32& ch)
{
	OIS::KeyCode result;

	bool translated = translateKeyBack(code, result);

	if (translated)
	{
		ch = 0;
		key = result;
	}
	else
	{
		ch = code;
		key = (OIS::KeyCode) 0;
	}
}

/**
	Returns an InputManager* on success or NULL on failure.
*/
OIS::InputManager* CreateInputManager(size_t ParentWindowHandle, bool showCursor)
{
	std::ostringstream ss;
	ss << ParentWindowHandle;
	std::string wnd_str = ss.str();

	OIS::ParamList pl;
	pl.insert(std::make_pair( std::string("WINDOW"), wnd_str));
	
	if (showCursor)
	{
#if defined(OIS_WIN32_PLATFORM)
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_FOREGROUND" )));
		pl.insert(std::make_pair(std::string("w32_mouse"), std::string("DISCL_NONEXCLUSIVE")));
#elif defined(OIS_LINUX_PLATFORM)
		pl.insert(std::make_pair(std::string("x11_mouse_grab"), std::string("false")));
		pl.insert(std::make_pair(std::string("x11_mouse_hide"), std::string("false")));
#else
		#error Implement OIS initialization code for this platform.
#endif
	}

	try
	{
		return OIS::InputManager::createInputSystem(pl);
	}
	catch (OIS::Exception& ex)
	{
		errlog << "OIS reported an error while initializing the input system: "
		       << ex.eFile << ":" << ex.eLine << ", "
		       << ex.eText;

		return NULL;
	}
}

void DestroyInputManager(OIS::InputManager* im)
{
	if (im)
	{
		OIS::InputManager::destroyInputSystem(im);
		im = NULL;
	}
}

} // namespace Utils
} // namespace Controller_
} // namespace BFG