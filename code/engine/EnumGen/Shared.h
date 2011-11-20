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

#ifndef BFG_ENUMGEN_SHARED_
#define BFG_ENUMGEN_SHARED_

#include <string>
#include <vector>
#include <iostream> // Shared::Enum::dump()

namespace EnumGen {

//! Shared data by Parse and Generator functions.
namespace Shared
{
	//! Equal to a C++ enum.
	/** Represents content of either the enumgen XML tag (enum) or
	    a C++ enum definition.
	*/
	struct Enum
	{
		//! Equal to a C++ enumerator-definition.
		/** Represents content of either the enumgen tag (value) or
		    an enumerator-defintion inside an C++ enumerator-list.
		    
		    Example: enum { FOO = BAR }; // FOO is Text, BAR is Set.
		*/
		struct ValueT
		{
			ValueT() : Set(0) {}
			
			std::string Text;
			
			int Set;
		};

		//! Additional information about the enum itself.
		struct MetaT
		{
			typedef std::vector<std::string> IncludeVector;

			MetaT() : DefinitionExists(false) {}
		
			//! If this is true, the enum definition won't get generated.
			/** It may be possible that the enum exists already in an other
			    library, but we need the conversion functions. This should
			    be true, if this is the case. This is indicated in XML by
			    the "definition-exists" attribute.
			*/
			bool DefinitionExists;

			//! If this is true, asXXX-functions will be exported.
			/** This variable contains the export macro which will be passed
			    to the compiler in order to produce the interface or the
			    library itself.
			*/
			std::string Export;

			//! Some enums may require special includes.
			/** This will even be considered if DefinitionExists is false. 
			*/
			IncludeVector Required;

			//! Of course, this enum may be put into some C++ namespace.
			/** Anonymous namespaces are not supported.
			*/
			std::string Namespace;
		};

		//! Writes debug output to std::cerr.
		void dump() const
		{
			std::cerr << "Shared::Enum dump for \'"<< mName <<"\'\n";
			
			for (size_t s=0; s<24+mName.size(); ++s) std::cerr << "*";
			std::cerr << "\n";

			for (size_t i=0; i<mValues.size(); ++i)
			{
				std::cerr << mValues[i].Text;
				
				if (mValues[i].Set)
					std::cerr << " = " << mValues[i].Set;
				
				std::cerr << "\n";
			}
			std::cerr << "\n"; 

			std::cerr << "Is this enum already defined: "
			          << (mMeta.DefinitionExists ? "Yes" : "No") << "\n\n";

			std::cerr << "Required includes for this enum:\n";
			for (size_t i=0; i<mMeta.Required.size(); ++i)
				std::cerr << " -> \"" << mMeta.Required[i] << "\"\n";
				
			if (mMeta.Required.empty())
				std::cerr << "<none>\n";
				
			std::cerr << "\n"; 
		}
		
		MetaT               mMeta;
		std::string         mName;
		std::vector<ValueT> mValues;
	};
	
	typedef std::vector<Enum> EnumVector;
}

} // namespace EnumGen

#endif