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

#include <Model/Loader/FileLoader.h>

#include <vector>
#include <string>
#include <boost/shared_ptr.hpp>
#include <boost/filesystem/operations.hpp>
#include <tinyxml.h>

#include <Base/CLogger.h>
#include <Model/Loader/Tags.h>

namespace BFG {
namespace Loader {

std::string printMap(Loader::ManyManyTagsT& map)
{
	std::stringstream ss;
	
	Loader::ManyManyTagsT::iterator itM = map.begin();
	for (; itM != map.end(); ++itM)
	{
		ss << "  " << itM->first << "\n";
		Loader::ManyTagsT::iterator itL = itM->second->begin();
		for (; itL != itM->second->end(); ++itL)
		{
			Loader::TagWithAttributesT::iterator itA = (*itL)->begin();
			for (; itA != (*itL)->end(); ++itA)
			{
				ss << "    " << itA->first << ": " << itA->second << "\n";
			}
			ss << "\n";
		}
	}
	return ss.str();
}

void FileLoader::loadSectorDraft(const std::string& filename, 
                                 SectorDraft& sectorDraft, 
                                 std::string& sectorName) const
{
	if (! boost::filesystem::exists(filename))
	{
		std::stringstream ss;
		ss << "FileLoader::loadSectorDraft: File \""
		   << filename << "\" does not exist.";
		
		throw std::runtime_error(ss.str());
	}

	TiXmlDocument document;
	document.LoadFile(filename);

	TiXmlElement* root = document.RootElement();
	if (!root)
	{
		throw std::runtime_error("FileLoader::loadMap Root not found!");
	}
	TiXmlHandle xmlHandle(root);

	sectorName = root->FirstAttribute()->Value();

	TiXmlElement* element = NULL;
	TiXmlAttribute* attribute = NULL;
	
	// parse objects
	element = xmlHandle.FirstChild(XmlTags::ObjectList)
	                   .FirstChild(XmlTags::Object)
	                   .ToElement();	

	if (element)
	{
		loadManyTags(element, sectorDraft.mObjects);
	}
	else
		warnlog << "FileLoader: No objects were found in " << filename;

	// parse raceTrack
	element = xmlHandle.FirstChild(XmlTags::PathList)
	                   .FirstChild(XmlTags::Path)
	                   .ToElement();

	if (element)
	{
		loadManyManyTags(element, XmlTags::Path,  sectorDraft.mPaths);
	}
	else
		warnlog << "FileLoader: No race track was found in " << filename;

	// parse raceConditions
	element = xmlHandle.FirstChildElement(XmlTags::Conditions)
	                   .ToElement();

	if (element)
	{
		attribute = element->FirstAttribute();
		loadTagWithAttributes(attribute, sectorDraft.mConditions);
	}
	else
		warnlog << "FileLoader: No race conditions were found in " << filename;

	// parse cameras
	element = xmlHandle.FirstChild(XmlTags::View)
	                   .FirstChild(XmlTags::Camera)
	                   .ToElement();

	if (element)
	{
		for (; element; element = element->NextSiblingElement(XmlTags::Camera))
		{
			boost::shared_ptr<TagWithAttributesT> camera(new TagWithAttributesT);
			attribute = element->FirstAttribute();
			
			loadTagWithAttributes(attribute, camera);
			sectorDraft.mCameras->push_back(camera);
		}
	}
	else
		warnlog << "FileLoader: No cameras were found in " << filename;

	// parse lights
	element = xmlHandle.FirstChild(XmlTags::View)
	                   .FirstChild(XmlTags::Light)
	                   .ToElement();

	if (element)
	{
		for (; element; element = element->NextSiblingElement(XmlTags::Light))
		{
			boost::shared_ptr<TagWithAttributesT> light(new TagWithAttributesT);
			attribute = element->FirstAttribute();

			loadTagWithAttributes(attribute, light);
			sectorDraft.mLights->push_back(light);
		}
	}
	else
		warnlog << "FileLoader: No lights were found in " << filename;

	// parse skybox
	element = xmlHandle.FirstChild(XmlTags::View)
	                   .FirstChild(XmlTags::Skybox)
	                   .ToElement();

	if (element)
	{
		attribute = element->FirstAttribute();
		loadTagWithAttributes(attribute, sectorDraft.mSkybox);
	}
	else
		warnlog << "FileLoader: No skybox was found in " << filename;


#if 0
	// parse sectorProperties
	element = xmlHandle.FirstChild(XmlTags::SectorProperties)
	                   .ToElement();
	attribute = element->FirstAttribute();

	loadTagWithAttributes(attribute, sectorDraft.mSectorProperties);
#endif

	// parse playlist
	element = xmlHandle.FirstChild(XmlTags::PlayList)
	                   .FirstChild(XmlTags::Song)
	                   .ToElement();

	if (element)
	{
		loadManyTags(element, sectorDraft.mPlaylist);
	}
	else
		warnlog << "FileLoader: No playlist was found in " << filename;
}

void FileLoader::loadManyManyTagFile(const std::string& filename,
                                     XmlTags::TagT& listTag,
                                     boost::shared_ptr<ManyManyTagsT> mmt) const
{
	assert(mmt);

	if (! boost::filesystem::exists(filename))
	{
		std::stringstream ss;
		ss << "FileLoader::loadManyManyTagFile: File \""
		   << filename << "\" does not exist.";
		
		throw std::runtime_error(ss.str());
	}

	TiXmlDocument document;
	document.LoadFile(filename);

	TiXmlElement* root = document.RootElement();
	if (!root)
	{
		throw std::runtime_error
			("FileLoader::loadManyManyTagFile: Root not found!");
	}
	
	TiXmlElement* element = root->FirstChildElement(listTag);
	if (!element)
	{
		warnlog << "FileLoader::loadManyManyTagFile: Root of file \""
		        << filename
		        << "\" has no elements!";
		return;
	}
	
	loadManyManyTags(element, listTag, mmt);
}

void FileLoader::loadManyManyTags(const TiXmlElement* element,
                                  XmlTags::TagT& listTag,
                                  boost::shared_ptr<ManyManyTagsT> mmt) const
{
	for (; element; element = element->NextSiblingElement(listTag))
	{
		// Name attribute is mandatory
		const std::string* name = element->Attribute(Tag::name);
		if (!name)
			throw std::runtime_error
				("FileLoader::loadMapOfEntityLists: No name attribute found "
				 "for \"" + listTag + "\"!");

		// Every name within a map must be unique
		ManyManyTagsT::const_iterator it = mmt->find(*name);
		if (it != mmt->end())
		{
			std::stringstream ss;
			ss << "FileLoader::loadManyManyTags: Found multiple defined tag "
			      " name \"" << *name << "\" in tag \"" << listTag << "\".";
			throw std::runtime_error(ss.str());
		}

		// Insert			
		boost::shared_ptr<ManyTagsT> mt(new ManyTagsT);

		if (!element->NoChildren())
		{
			const TiXmlElement* child = element->FirstChild()->ToElement();

			loadManyTags(child, mt);
		}

		mmt->insert(ManyManyTagsT::value_type(*name, mt));
	}
}

void FileLoader::loadManyTags(const TiXmlElement* element,
                              boost::shared_ptr<ManyTagsT> mt) const
{
	for (; element; element = element->NextSiblingElement())
	{
		boost::shared_ptr<TagWithAttributesT> twa(new TagWithAttributesT);

		const TiXmlAttribute* attribute = element->FirstAttribute();		
		loadTagWithAttributes(attribute, twa);

		mt->push_back(twa);
	}
}

void FileLoader::loadTagWithAttributes(const TiXmlAttribute* attribute,
                                       boost::shared_ptr<TagWithAttributesT> twa) const
{
	for (; attribute; attribute = attribute->Next())
	{
		// Insert
		twa->insert
		(
			TagWithAttributesT::value_type
			(
				attribute->Name(),
				attribute->ValueStr()
			)
		);
	}
}

} // namespace Loader
} // namespace BFG
