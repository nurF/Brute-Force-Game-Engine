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

#ifndef TESTAPP_DECO_H
#define TESTAPP_DECO_H

#include <iostream>

namespace BFG {
namespace Base {
namespace Deco {

namespace detail {

void h(const std::string& text, char sign, size_t textLength)
{
	std::cout << text << "\n";
	std::cout << std::string(textLength, sign) << "\n";
	std::cout << "\n";
}

void h(const std::string& text, char sign)
{
    h(text, sign, text.length());
}

} // namespace detail

// So this is decorational stuff for our test applications.
// You remember tags like <h1>Caption</h1>? Good. ^^

void h1(const std::string& text)
{
	detail::h(text, '#');
}

void h2(const std::string& text)
{
	detail::h(text, '=');
}

void h3(const std::string& text)
{
	detail::h(text, '-');
}

void h3(const std::string& text, size_t width)
{
	detail::h(text, '-', width);
}

} // namespace Deco
} // namespace Base
} // namespace BFG

#endif