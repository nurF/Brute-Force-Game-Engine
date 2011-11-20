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

#include <Pong/Score.h>

#include <MyGUI.h>

#include <EventSystem/Core/EventLoop.h>
#include <View/Main.h>

#include <Pong/PongDefinitions.h>

namespace BFG {
namespace View {

Score::Score() :
mUpperScore(0),
mLowerScore(0),
HudElement("PongScore.layout", "Score")
{
	Main::eventLoop()->connect
	(
		A_UPPER_BAR_WIN,
		this,
		&Score::viewEventHandler
	);

	Main::eventLoop()->connect
	(
		A_LOWER_BAR_WIN,
		this,
		&Score::viewEventHandler
	);

	MyGUI::Gui& gui = MyGUI::Gui::getInstance();
	gui.setVisiblePointer(false);
	setVisible(true);
}

Score::~Score()
{
	setVisible(false);
	Main::eventLoop()->disconnect(A_UPPER_BAR_WIN, this);
	Main::eventLoop()->disconnect(A_LOWER_BAR_WIN, this);
}

void Score::internalUpdate(f32 time)
{
}

void Score::viewEventHandler(Event* VE)
{
	switch(VE->getId())
	{
	case A_UPPER_BAR_WIN:
	{
		++mUpperScore;
		onScoreUpdate();
		break;
	}
	case A_LOWER_BAR_WIN:
	{
		++mLowerScore;
		onScoreUpdate();
		break;
	}
	}
}

void Score::onScoreUpdate()
{
	MyGUI::Gui& gui = MyGUI::Gui::getInstance();

	MyGUI::StaticText* upper = gui.findWidget<MyGUI::StaticText>("UpperScore");
	MyGUI::StaticText* lower = gui.findWidget<MyGUI::StaticText>("LowerScore");

	std::stringstream ss;
	ss << mUpperScore;
	upper->setCaption(ss.str());

	ss.clear();
	ss.str("");

	ss << mLowerScore;
	lower->setCaption(ss.str());
}

} // namespace View
} // namespace BFG
