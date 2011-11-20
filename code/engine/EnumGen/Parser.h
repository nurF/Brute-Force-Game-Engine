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

#include <iostream>   // For warnings. (std::cerr)
#include <boost/algorithm/string/case_conv.hpp>   // tolower()
#include <tinyxml.h>

#include "Shared.h"
#include "Exception.h"

namespace EnumGen {

namespace Program
{
	const std::string DefaultFile = "Enums.xml";
	const std::string Version = "4";
}

//! Functions which use TinyXML to parse XML input and fill "Shared" structs.
namespace Parse
{
	//! XML specification: Tag identifiers.
	namespace Tag
	{
		const std::string Root    = "enumgen";
		const std::string Enum    = "enum";
		const std::string Value   = "value";
		const std::string Include = "include";
		const std::string Namespace = "namespace";
	}

	//! XML specification: Attribute identifiers.
	namespace Attr
	{
		const std::string Version          = "version";
		const std::string Name             = "name";
		const std::string DefinitionExists = "definition-exists";
		const std::string Set              = "set";
		const std::string Export           = "export";
	}

	namespace detail
	{
		void AddValues(TiXmlElement* Value, Shared::Enum& Storage)
		{
			do
			{
				Shared::Enum::ValueT Val;

				const char* Text = Value->GetText();
				assert(Text);
				Val.Text = Text;

				// If this returns something, Val.Set will get written to.
				const std::string* Set = Value->Attribute(Attr::Set, &Val.Set);

				// Otherwise we better set it back to zero...
				if (! Set) {
					Val.Set = 0;
				}
				
				Storage.mValues.push_back(Val);
			}
			while (Value = Value->NextSiblingElement(Tag::Value));
		}

		void AddIncludes(TiXmlElement* Value, Shared::Enum& Storage)
		{
			do
			{
				const char* Text = Value->GetText();
				assert(Text);
				
				Storage.mMeta.Required.push_back(Text);
			}
			while (Value = Value->NextSiblingElement(Tag::Include));
		}

		void AddEnums(TiXmlElement* currentTag,
		              Shared::EnumVector& Everything,
		              const std::string Namespace)
		{
			do
			{
				Shared::Enum Storage;
				
				// Attribute: Name
				const std::string* Name = currentTag->Attribute(Attr::Name);
			
				if (! Name)
				{
					std::cerr << "Warning: Skipping unnamed enum."
					          << std::endl;
					continue;
				}
				else
					Storage.mName = *Name;

				// Attribute: Definition exists
				const std::string* Exists;
				Exists = currentTag->Attribute(Attr::DefinitionExists);
				
				if (Exists)
				{
					std::string Copy(*Exists);
					boost::algorithm::to_lower(Copy);
					Storage.mMeta.DefinitionExists = (Copy == "yes");
				}
				
				// Attribute: Export
				const std::string* Exports;
				Exports = currentTag->Attribute(Attr::Export);
				
				if (Exports)
				{
					Storage.mMeta.Export = *Exports;
				}
					
				// <include>
				TiXmlElement* Include = currentTag->FirstChildElement(Tag::Include);

				if (Include)
					detail::AddIncludes(Include, Storage);

				// <value>
				TiXmlElement* Value = currentTag->FirstChildElement(Tag::Value);

				if (Value)
					detail::AddValues(Value, Storage);
				else
					std::cerr << "Warning: No values inside of enum."
					          << std::endl;
				
				Storage.mMeta.Namespace = Namespace;

				Everything.push_back(Storage);
			}
			while (currentTag = currentTag->NextSiblingElement(Tag::Enum));
		}

		void AddNamespaces(TiXmlElement* currentTag,
		                   Shared::EnumVector& Everything)
		{
			do
			{
				// We might find some <Enum> tags now.
				// Well, read them all without namespace and carry on.
				if (currentTag->Value() == Tag::Enum)
				{
					AddEnums(currentTag, Everything, "");
					continue;
				}

				if (currentTag->Value() == Tag::Namespace)
				{
					// Attribute: Name
					const std::string* Name = currentTag->Attribute(Attr::Name);
				
					if (! Name)
					{
						std::cerr << "Warning: Skipping anonymous namespace."
								  << std::endl;
						continue;
					}

					currentTag = currentTag->FirstChildElement(Tag::Enum);
					AddEnums(currentTag, Everything, *Name);
				}
			}
			while (currentTag = currentTag->NextSiblingElement(Tag::Namespace));
		}
		              
	} // namespace detail

	//! Checks enumgen header and version.
	//! \return Handle to the <enumgen> node
	TiXmlHandle Header(TiXmlHandle hDoc)
	{
		TiXmlElement* E = hDoc.FirstChildElement().Element();

		if (E->Value() != Tag::Root)
			throw Exception::WrongFormat();

		const std::string* VersionAttribute = E->Attribute(Attr::Version);
		
		if (! VersionAttribute)
			throw Exception::FileCorrupt();

		if (Program::Version != *VersionAttribute)
			throw Exception::VersionConflict(Program::Version,
			                                 *VersionAttribute);	

		return TiXmlHandle(E);
	}

	void Content(TiXmlHandle hRoot, Shared::EnumVector& Everything)
	{	
		TiXmlElement* FirstTag = hRoot.FirstChild().Element();

		if (! FirstTag)
			throw Exception::NoEntriesFound();
			
		detail::AddNamespaces(FirstTag, Everything);
	}
}

} // namespace EnumGen