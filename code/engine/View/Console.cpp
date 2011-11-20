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

#include <View/Console.h>

#include <EventSystem/Core/EventLoop.h>

#include <boost/log/common.hpp>
#include <boost/log/formatters/message.hpp>
#include <boost/log/formatters/stream.hpp>
#include <boost/log/formatters/attr.hpp>
#include <boost/log/attributes/value_extraction.hpp>
#include <MyGUI_TextIterator.h>

#include <Core/Path.h>

#include <View/HudElement.h>
#include <View/Event.h>

namespace BFG{
namespace View{

Console::Console(EventLoop* Loop) :
HudElement("Console.layout", "Console"),
Emitter(Loop),
mHasNewContent(false),
mIsVisible(false),
mLoop(Loop)
{
	createUI();
	registerSink();
}

Console::~Console()
{
	boost::log::core::get()->remove_sink(mSink);
}

void Console::createUI()
{ 
	using namespace MyGUI;

	Gui& gui = Gui::getInstance();

	mStaticText = gui.findWidget<StaticText>("ConsoleText");
	mEdit = gui.findWidget<Edit>("ConsoleEdit");

	this->setVisible(false);

	//binding events
	mEdit->eventEditSelectAccept = newDelegate(this, &Console::input);
}

namespace logging = boost::log;
namespace fmt = boost::log::formatters;

void colorFormatter(std::ostream& strm, logging::record const& rec)
{
	Base::Logger::SeverityLevel sl = logging::extract<Base::Logger::SeverityLevel>
	(
		"Severity",
		rec.attribute_values()
	).get();
	
	if      (sl == Base::Logger::SL_DEBUG)       strm << "#000080";
	else if (sl == Base::Logger::SL_INFORMATION) strm << "#FFFFFF";
	else if (sl == Base::Logger::SL_WARNING)     strm << "#FFFF00";
	else if (sl == Base::Logger::SL_ERROR)       strm << "#FF0000";

    strm << rec.message();
}

void Console::registerSink()
{
	// Construct the sink
    mSink = boost::make_shared<text_sink>();
	
    // Add a stream to write log to
    mSink->locked_backend()->add_stream(boost::make_shared<std::ostream>(this));
	mSink->locked_backend()->set_formatter(&colorFormatter);

    // Register the sink in the logging core
	boost::log::core::get()->add_sink(mSink);
}

void Console::toggleVisible(bool show)
{
	this->setVisible(show);
	mIsVisible = show;
}

void Console::internalUpdate(f32)
{
	if (mHasNewContent && mIsVisible)
	{	
		mStaticText->setCaption(mConsoleContent);
		mHasNewContent = false;
	}
}

//! Adds the Text of the Inputfield to the output field, 
//! and clears the input field
void Console::input(MyGUI::Widget * Sender)
{
	std::string command = mEdit->getCaption();
	emit<Event>(BFG::ID::VE_CONSOLE_COMMAND, command);

	mConsoleContent += command + "\n";
 	mStaticText->setCaption(mConsoleContent);
 	mEdit->eraseText(0, -1);
}

//! virtual function of std::streambuf, is expected to display c
int Console::overflow(int c)
{
	mConsoleContent += c;
	mHasNewContent = true;
	return 0;
}

//! virtual function of std::streambuf, is expected to display the first n
//! letters in s
int Console::xsputn(const char * s, int n)
{
	mConsoleContent += std::string(s, n);
	mHasNewContent = true;
	return n;
}

}}

