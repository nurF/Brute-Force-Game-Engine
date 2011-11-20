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

namespace EnumGen {

namespace Exception
{
	/* Seems like there's no td::exception(const char*) overload
	   in g++ 4.3.4. 
	*/
	struct ExBase : std::exception
	{
		ExBase(const std::string& Description) :
			Description(Description)
		{
		}

		ExBase() :
			Description("EnumGen exception")
		{
		}

		virtual ~ExBase() throw()
		{
		}

		virtual const char* what() const throw()
		{
			return Description.c_str();
		}
		

		std::string Description;
	};

	struct FileNotFound : ExBase
	{
		FileNotFound(const std::string& Filename) :
			ExBase
			(
				std::string("File \"")
				+ Filename
				+ std::string("\" not found.")
			)
		{
		}
	};

	struct WrongFormat : ExBase
	{
		WrongFormat() :
			ExBase("Wrong format")
		{
		}
	};

	struct VersionConflict : ExBase
	{
		VersionConflict(const std::string& Required,
		                const std::string& Found) :
			ExBase
			(
				std::string("Required enumgen version is ")
				+ Required
				+ std::string(", but got ")
				+ Found
				+ std::string(".")
			)
		{
		}
	};

	struct FileCorrupt : ExBase
	{
		FileCorrupt() :
			ExBase(std::string("XML file corrupt"))
			{
			}
	};
	
	struct NoEntriesFound : ExBase
	{
		NoEntriesFound() :
			ExBase(std::string("No entries found"))
			{
			}
	};
}

} // namespace EnumGen