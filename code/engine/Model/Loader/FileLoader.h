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

#ifndef FILE_LOADER_H_
#define FILE_LOADER_H_

#include <Model/Loader/Types.h>
#include <Model/Loader/Tags.h>

class TiXmlDocument;
class TiXmlElement;
class TiXmlAttribute;

namespace BFG {
namespace Loader {

//! Generates debug output
std::string printMap(Loader::ManyManyTagsT& map);

//! \brief
//!		This class parses XML files in a generic way and puts its contents
//!     into nested structures.
/**
	The parsing hierarchy looks like this:\n
	\n
	\verbatim
	<Config>
	    <Object name = "Racer">     <- ManyManyTagsT
	        <Module                 <- ManyTagsT
	            name = "Cockpit"    <- TagWithAttributesT (first = "second")
	            mesh = "Racer.mesh" <- TagWithAttributesT (first = "second")
	        />
	    />
	</Config>
	\endverbatim
	\n
	A TagWithAttributesT contains all attributes of a tag. It's a map:
	'first' will be the attribute name, 'second' the attribute value.\n
	\n
	The ManyTagsT would represent the content of a tag like "Object", in this
	case: Many tags like "Module".
	It's a vector of multiple "TagWithAttributesT".\n
	\n
	The ManyManyTagsT would represent the content of many tags like "Object".
	Since it is a Map, 'first' will be "Racer", 'second' will be a pointer to
	a ManyTags type.
*/
class FileLoader
{

public:
	//! Loads a complete sector (level)
	//! \todo This does not belong into the engine
	void loadSectorDraft(const std::string& filename, 
	                     SectorDraft& outSectorDraft, 
	                     std::string& outSectorName) const;

	void loadManyManyTagFile(const std::string& filename,
	                         XmlTags::TagT& listTag,
	                         boost::shared_ptr<ManyManyTagsT> mmt) const;

	void loadManyManyTags(const TiXmlElement* firstElement, 
	                      XmlTags::TagT& listTag,
	                      boost::shared_ptr<ManyManyTagsT> mmt) const;

	void loadManyTags(const TiXmlElement* firstElement,
	                  boost::shared_ptr<ManyTagsT> mt) const;

	void loadTagWithAttributes(const TiXmlAttribute* firstAttribute,
	                           boost::shared_ptr<TagWithAttributesT> twa) const;
};

} // Loader
} // namespace BFG

#endif
