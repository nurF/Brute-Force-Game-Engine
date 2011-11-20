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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <View/HudElement.h>

#include <boost/log/sinks.hpp>
#include <EventSystem/Emitter.h>

#include <MyGUI.h>
#include <iostream>

class EventLoop;

namespace BFG {
namespace View {


//! The Class is derived from std::streambuf, so it's possible to redirect
//! the logger output.
class Console : public HudElement,
                private Emitter,
                public std::streambuf
{
public:
	Console(EventLoop*);
	~Console();
	
	// Those Methods are eventhandler methods

	//! \brief Hides or Shows the Console
	//! \brief Is called by the Event ID::A_CONSOLE
	void toggleVisible(bool show);

	//! \brief Is called, when someone presses enter in the inputline
	void input(MyGUI::Widget*);

protected:
	//! \brief Initiates the UserInterface of the Console
	void createUI();

	/** @brief	this 2 methods are part of std::streambuf 
				and have to be overwritten.
	*/
	
	//! @{
	int xsputn ( const char * s, int n );
	int overflow(int);
	//! @}


	//Widgets:
	MyGUI::StaticTextPtr mStaticText;
	MyGUI::EditPtr mEdit;
	MyGUI::WidgetPtr mWin;

	bool mHasNewContent;
	bool mIsVisible;

	std::string mConsoleContent;

	EventLoop* mLoop;

private:
	//! \brief Used to connect the console Outputfield with the Logger.
	void registerSink();
	void internalUpdate(f32);
	
	typedef boost::log::sinks::asynchronous_sink<boost::log::sinks::text_ostream_backend> text_sink;
	boost::shared_ptr<text_sink> mSink;

};

}}

#endif
