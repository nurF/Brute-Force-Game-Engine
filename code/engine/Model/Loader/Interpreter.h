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
#ifndef INTERPRETER_H_
#define INTERPRETER_H_

#include <Model/Loader/Types.h>
#include <Model/Loader/SectorFactory.h>
#include <Model/Loader/Exceptions.h>
#include <Model/Property/Plugin.h>

namespace BFG {

class Environment;

namespace Loader {

//! \brief Utility function for Interpreter
//! Not all types of the GoePayloadT are supported though.
//! \exception std::runtime_error If the input couldn't be converted. 
Property::Value MODEL_API StringToPropertyValue(const std::string& input);

//! \brief Utility function for Interpreter
//! \param[in] input Example: "Wing:5" or "Body:1". Both parts are mandatory.
//! \param[out] connection Result of parsing
//! \exception std::runtime_error If the input couldn't be parsed. 
void MODEL_API parseConnection(const std::string& input,
                               Connection& connection);

//! \brief Serializes string data into engine structures
class MODEL_API Interpreter
{
public:
	Interpreter(const Property::PluginMapT& pm);

	void interpretConceptConfig(const ManyTagsT& definitions,
	                            std::vector<ConceptParameter>& conceptVector) const;
	                            
	void interpretConceptDefinition(const TagWithAttributesT& definitions,
	                                ConceptParameter& conceptParameters) const;

	void interpretValueConfig(const ManyTagsT& definitions,
	                          std::vector<ValueParameter>& propertyVector) const;

	void interpretValueDefinition(const TagWithAttributesT& definitions,
	                              ValueParameter& valueParameters) const;

	void interpretAdapterConfig(const ManyTagsT& definitions,
	                            std::vector<AdapterParameter>& adapterVector) const;

	void interpretAdapterDefinition(const TagWithAttributesT& definitions,
	                                AdapterParameter& adapterParameters) const;

	void interpretModuleConfig(const ManyTagsT& definitions,
	                            std::vector<ModuleParameter>& moduleVector) const;

	void interpretModuleDefinition(const TagWithAttributesT& definitions,
	                               ModuleParameter& moduleParameters) const;
	                               
	void interpretObjectConfig(const ManyTagsT& definitions,
	                           std::vector<ObjectParameter>& objectVector) const;

	void interpretObjectDefinition(const TagWithAttributesT& definitions,
	                               ObjectParameter& objectParameters) const;

#if 0
	void interpretRaceCondition(const TagWithAttributesT& definitions,
	                            RaceCondition& raceCondition,
	                            std::string& path) const;
#endif
	                            
	void interpretLightDefinition(const TagWithAttributesT& definitions,
	                              LightParameters& lightParameters) const;
	                              
	void interpretCameraDefinition(const TagWithAttributesT& definitions,
	                               CameraParameter& cameraParameters,
	                               std::string& objectName) const;
	                               
	void interpretPlaylistDefinition(const TagWithAttributesT& definitions,
	                                 std::string& publicName,
	                                 std::string& filename) const;

#if 0	                                 
	GameHandle interpretPathDefinition(const std::string& waypointDefiniton,
	                                   SectorFactory& sectorFactory) const;	
#endif

private:
	Property::PluginMapT mPropertyPlugins;	

	bool grab(const std::string& tag,
	          const TagWithAttributesT& container,
	          std::string& out,
	          bool optional = false) const;

	template <typename targetT>
	void interpret(const std::string& in, targetT& out) const;
	void interpret(const std::string& in, s32& out) const;

	template<typename targetT>
	void convert(const std::string& in, targetT& out) const;
	void convert(const std::string& in, qv4& out) const;
	void convert(const std::string& in, v3& out) const;
	void convert(const std::string& in, bool& out) const;
};

} // namespace Loader
} // namespace BFG

#endif
