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

#include <Core/Path.h>

#include <boost/filesystem.hpp>
#include <boost/noncopyable.hpp>

#include <tinyxml.h>

namespace BFG {

namespace detail {


//! Visitor callback for TinyXml
template <typename ContainerT>
class PathVisitor : public TiXmlVisitor, boost::noncopyable
{
public:
	PathVisitor(ContainerT& all) :
	mAll(all)
	{
	}

	virtual bool VisitEnter (const TiXmlElement& e, const TiXmlAttribute*) 
	{
		if (e.ValueStr() == "path")
		{
			ID::PathType type = ID::asPathType(e.Attribute("type"));
			const std::string* location = e.Attribute(std::string("location"));
			const std::string* create(e.Attribute(std::string("autocreate")));
			bool autocreate = false;
			if (create && *create == "true")
			{
				autocreate = true;
			}
			
			mAll.insert
			(
				std::make_pair
				(
					type,
					PathProperties(*location, autocreate)
				)
			);
		}
		return true;
	}

private:
	ContainerT& mAll;
};

//! External loader component for Path.
//! This one uses Xml.
struct PathLoader
{
	template <typename ContainerT>
	static void load(ContainerT& mAll)
	{
		const std::string file("Paths.xml");
	
		PathVisitor<ContainerT> pv(mAll);
		TiXmlDocument doc(file);
		doc.LoadFile();
		doc.Accept(&pv);

		if (doc.Error())
		{
			std::stringstream ss;
			ss << "Path: Error while parsing \"" << file
			   << "\" (TiXml: " << doc.ErrorDesc()
			   << ")" << std::endl;
			throw std::runtime_error(ss.str());
		}
	}
};

} // namespace detail



Path::Path()
{
	load<detail::PathLoader>();
}

std::string Path::Expand(const std::string& Filename) const
{
	ID::PathType pt = search(Filename);
	return Get(pt) + Filename;
}

std::string Path::Get(ID::PathType pt) const
{
	ContainerT::const_iterator it = mAll.find(pt);

	if (it == mAll.end())
	{
		std::stringstream ss;
		ss << "PathType \"" << ID::asStr(pt)
		   << "\" not found." << std::endl;
		   
		throw std::logic_error(ss.str());
	}

	return it->second.mLocation+"/";
}

template <typename Loader>
void Path::load()
{
	Loader::load(mAll);
	checkPaths();
}

void Path::checkPaths() const
{
	ContainerT::const_iterator it = mAll.begin();
	for (; it != mAll.end(); ++it)
	{
		if (! boost::filesystem::exists(it->second.mLocation))
		{
			if (it->second.mAutocreate)
			{
				if(! boost::filesystem::create_directory(it->second.mLocation))
				{
					std::stringstream ss;
					ss << "The directory \"" << it->second.mLocation
						<< "\" doesn't exist and couldn't be created.";

					throw std::runtime_error(ss.str());
				}
			}
			else
			{
				std::stringstream ss;
				ss << "The directory \"" << it->second.mLocation
					<< "\" doesn't exist.";

				throw std::runtime_error(ss.str());
			}
		}
		else if (! boost::filesystem::is_directory(it->second.mLocation))
		{
			std::stringstream ss;
			ss << "\"" << it->second.mLocation
			   << "\" is no directory";

			throw std::runtime_error(ss.str());
		}
	}
}

ID::PathType Path::search(const std::string& Filename) const
{
	ContainerT::const_iterator it = mAll.begin();
	for (; it != mAll.end(); ++it)
	{
		const std::string path = it->second.mLocation + "/" + Filename;
		if (boost::filesystem::exists(path))
			return it->first;
	}

	std::stringstream ss;
	ss << "Path: File \"" << Filename
	   << "\"" << " not found.";

	throw std::runtime_error(ss.str());
}

void levelDirectories(std::vector<std::string>& result)
{
	Path p;
	const std::string location = p.Get(ID::P_SCRIPTS_LEVELS);

	boost::filesystem::directory_iterator it(location), end;
	
	for (; it != end; ++it)
	{
		const std::string directoryName = it->path().filename().string();
	
		// Skip non-directories
		if (! boost::filesystem::is_directory(*it))
			continue;

		// Skip hidden directories (e.g. ".svn")
		if (directoryName[0] == '.')
			continue;

		// Skip "default" level directory
		if (directoryName == "default")
			continue;
			
		// Skip directories without Sector.xml file
		const std::string curSectorXml = location
		                                 + directoryName + "/"
		                                 + "Sector.xml";
		
		if (! boost::filesystem::exists(curSectorXml))
			continue;

		result.push_back(it->path().filename().string());
	}
}


} // namespace BFG