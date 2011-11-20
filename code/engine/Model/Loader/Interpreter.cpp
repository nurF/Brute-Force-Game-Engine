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

#include <Model/Loader/Interpreter.h>

// In order to make this work for Visual Studio 2008, we disabled (/Zc:wchar_t).
// If you want to dig deeper into the problem, look out for
// BOOST_REGEX_HAS_OTHER_WCHAR_T in <boost/regex/config.hpp> 
#include <boost/regex.hpp>

#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <sstream>

#include <Base/CLogger.h>

#include <Model/Loader/Exceptions.h>
#include <Model/Loader/Tags.h>
#include <Model/Property/Concepts/Camera.h> // struct CameraParameter

namespace BFG {
namespace Loader {

static void print(boost::shared_ptr<TagWithAttributesT> attributes)
{
	BOOST_FOREACH(TagWithAttributesT::value_type vt, *attributes)
	{
		errlog << "* \"" << vt.first << "\" = \"" << vt.second << "\"";
	}
}

Property::Value StringToPropertyValue(const std::string& input)
{
	Property::Value result;
	
	// is qv4
	try {
		qv4 output;
		stringToQuaternion4(input, output);
		result = output;
		return result;
	}
	catch (std::runtime_error) {}
	
	// is v3
	try {
		v3 output;
		stringToVector3(input, output);
		result = output;
		return result;
	}
	catch (std::runtime_error) {}

	// is integer
	try
	{
		result = boost::lexical_cast<s32>(input);
		return result;
	}
	catch (boost::bad_lexical_cast &) {}

	// is float
	try
	{
		result = boost::lexical_cast<f32>(input);
		return result;
	}
	catch (boost::bad_lexical_cast &) {}

	// is ID::CameraMode
	try {
		result = ID::asCameraMode(input);
		return result;
	}
	catch (std::out_of_range) {}
	
	// is bool
	if (boost::iequals(input, "yes") || boost::iequals(input, "true"))
	{
		result = true;
		return result;
	}
	else if (boost::iequals(input, "no") || boost::iequals(input, "false"))
	{
		result = false;
		return result;
	}

	result = stringToArray<128>(input);
	return result;
}

void parseConnection(const std::string& input,
                     Connection& connection)
{
	if (input.empty())
		return;

	static const std::string rgx(
		"("
			"[[:digit:]]+"
		")"
		"@"
		"(?:"
			"("
				"[^:]+"
			")"
			":"
		")?"
		"("
			"[^:]+"
		")"
		":"
		"("
			"[[:digit:]]+"
		")"
	);

	// This is thread safe when BOOST_HAS_THREADS is defined.
	static const boost::regex e(rgx);

	boost::smatch what;
	if (boost::regex_match(input, what, e))
	{
		connection.mConnectedLocalAt = boost::lexical_cast<u32>(what[1]);
		connection.mConnectedExternToGameObject = what[2];
		connection.mConnectedExternToModule = what[3];
		connection.mConnectedExternAt = boost::lexical_cast<u32>(what[4]);
	}
	
	// Check and return if good
	if (connection.good())
		return;

	std::stringstream err;
	err << "parseConnection: Unable to parse \"" << input << "\"!";
	throw std::runtime_error(err.str());
}

Interpreter::Interpreter(const Property::PluginMapT& pm) :
mPropertyPlugins(pm)
{
	assert(! pm.empty() &&
		"Not a good idea to have support for no properties at all, or is it?");
}

void Interpreter::interpretValueConfig(const ManyTagsT& definitions,
                                       std::vector<ValueParameter>& propertyVector) const
{
	ManyTagsT::const_iterator it = definitions.begin();
	for (; it != definitions.end(); ++it)
	{
		boost::shared_ptr<TagWithAttributesT> attrMap = *it;

		try
		{
			ValueParameter parameter;
			interpretValueDefinition(*attrMap, parameter);
			
			propertyVector.push_back(parameter);
		}
		catch (std::exception& e)
		{
			errlog << "Loader::Interpreter::interpretValueConfig: "
			          "Error while parsing parameters:";

			print(attrMap);

			errlog << "Exception: " << e.what();
		}
	}
}

void Interpreter::interpretValueDefinition(const TagWithAttributesT& definitions,
                                           ValueParameter& valueParameter) const
{
	std::string buffer;
	
	grab(Tag::id, definitions, buffer);

	valueParameter.mId = symbolToValueId(buffer, mPropertyPlugins);

	grab(Tag::value, definitions, buffer);
	valueParameter.mValue = StringToPropertyValue(buffer);
}

void Interpreter::interpretConceptConfig(const ManyTagsT& definitions,
										 std::vector<ConceptParameter>& conceptVector) const
{
	ManyTagsT::const_iterator it = definitions.begin();
	for (; it != definitions.end(); ++it)
	{
		boost::shared_ptr<TagWithAttributesT> attrMap = *it;
		
		try
		{
			ConceptParameter parameter;
			interpretConceptDefinition(*attrMap, parameter);

			conceptVector.push_back(parameter);
		}
		catch (std::exception& e)
		{
			errlog << "Loader::Interpreter::interpretConceptConfig: "
			          "Error while parsing parameters:";

			print(attrMap);

			errlog << "Exception: " << e.what();
		}
	}
}

void Interpreter::interpretConceptDefinition(const TagWithAttributesT& definitions,
											 ConceptParameter& conceptParameters) const
{
	std::string buffer;

	grab(Tag::id, definitions, buffer);
	
	interpret(buffer, conceptParameters.mId);
	
	grab(Tag::properties, definitions, conceptParameters.mProperties);
}

void Interpreter::interpretAdapterConfig(const ManyTagsT& definitions,
                                         std::vector<AdapterParameter>& adapterVector) const
{
	ManyTagsT::const_iterator it = definitions.begin();
	for (; it != definitions.end(); ++it)
	{
		boost::shared_ptr<TagWithAttributesT> attrMap = *it;

		try
		{
			AdapterParameter parameter;
			interpretAdapterDefinition(*attrMap, parameter);

			adapterVector.push_back(parameter);
		}
		catch (std::exception& e)
		{
			errlog << "Loader::Interpreter::interpretAdapterConfig: "
			          "Error while parsing parameters:";

			print(attrMap);

			errlog << "Exception: " << e.what();
		}
	}
}

void Interpreter::interpretAdapterDefinition(const TagWithAttributesT& definitions,
                                             AdapterParameter& adapterParameters) const
{
	std::string buffer;
	
	grab(Tag::id, definitions, buffer);
	interpret(buffer, adapterParameters.mId);
	
	grab(Tag::position, definitions, buffer);
	interpret(buffer, adapterParameters.mLocation.position);
	
	grab(Tag::orientation, definitions, buffer);
	stringToQuaternion4(buffer, adapterParameters.mLocation.orientation);
}

void Interpreter::interpretModuleConfig(const ManyTagsT& definitions,
                                        std::vector<ModuleParameter>& moduleVector) const
{
	ManyTagsT::const_iterator it = definitions.begin();
	for (; it != definitions.end(); ++it)
	{
		boost::shared_ptr<TagWithAttributesT> attrMap = *it;

		try
		{
			ModuleParameter parameter;
			interpretModuleDefinition(*attrMap, parameter);

			moduleVector.push_back(parameter);
		}
		catch (std::exception& e)
		{
			errlog << "Loader::Interpreter::interpretModuleConfig: "
			          "Error while parsing parameters:";

			print(attrMap);

			errlog << "Exception: " << e.what();
		}
	}
}

void Interpreter::interpretModuleDefinition(const TagWithAttributesT& definitions,
                                            ModuleParameter& moduleParameters) const
{
	std::string buffer;
	
	grab(Tag::name, definitions, moduleParameters.mName);
	grab(Tag::mesh, definitions, moduleParameters.mMeshName, true);
	
	grab(Tag::connection, definitions, buffer, true);
	parseConnection(buffer, moduleParameters.mConnection);

	grab(Tag::adapters, definitions, moduleParameters.mAdapters, true);
	grab(Tag::concepts, definitions, moduleParameters.mConcepts);

	grab(Tag::collision, definitions, buffer, true);
	moduleParameters.mCollisionMode = ID::asCollisionMode(buffer);
	
	grab(Tag::visible, definitions, buffer, true);
	interpret(buffer, moduleParameters.mVisible);
	
	grab(Tag::density, definitions, buffer, true);
	interpret(buffer, moduleParameters.mDensity);
}

void Interpreter::interpretObjectConfig(const ManyTagsT& definitions,
						                std::vector<ObjectParameter>& objectVector) const
{
	ManyTagsT::const_iterator it = definitions.begin();
	for (; it != definitions.end(); ++it)
	{
		boost::shared_ptr<TagWithAttributesT> attrMap = *it;

		try
		{
			ObjectParameter parameter;
			interpretObjectDefinition(*attrMap, parameter);

			objectVector.push_back(parameter);
		}
		catch (std::exception& e)
		{
			errlog << "Loader::Interpreter::interpretObjectConfig: "
			          "Error while parsing parameters:";

			print(attrMap);

			errlog << "Exception: " << e.what();
		}
	}
}

void Interpreter::interpretObjectDefinition(const TagWithAttributesT& definitions,
                                            ObjectParameter& objectParameters) const
{
	std::string bufferString;

	grab(Tag::name, definitions, objectParameters.mName);
	grab(Tag::type, definitions, objectParameters.mType);

	grab(Tag::position, definitions, bufferString);
	interpret(bufferString, objectParameters.mLocation.position);

	grab(Tag::orientation, definitions, bufferString);
	interpret(bufferString, objectParameters.mLocation.orientation);
	
	grab(Tag::linear_velocity, definitions, bufferString, true);
	interpret(bufferString, objectParameters.mLinearVelocity);

	grab(Tag::angular_velocity, definitions, bufferString, true);
	interpret(bufferString, objectParameters.mAngularVelocity);
	
	grab(Tag::connection, definitions, bufferString, true);
	parseConnection(bufferString, objectParameters.mConnection);
}

#if 0
GameHandle Interpreter::interpretPathDefinition(const std::string& waypointDefiniton, 
                                                SectorFactory& sectorFactory) const
{
	std::string copyDefinition = waypointDefiniton;

	size_t signPosition = copyDefinition.find_first_of(Sign::interrupt);

	if (signPosition == std::string::npos)
	{
		std::stringstream strStream;
		strStream << "Waypoint definition misses" << Sign::interrupt << "sign to separate prefix.";
		throw LoaderException(strStream.str());
	}

	std::string prefix = copyDefinition.substr(0, signPosition);
	std::string mainString = copyDefinition.substr(signPosition +1, copyDefinition.size());

	if (prefix == Prefix::object)
	{
		return sectorFactory.createWaypoint(mainString);
	}
	else
	if (prefix == Prefix::position)
	{
		infolog << "Absolute waypoint with position prefix is not implemented atm.";
 		v3 position;
 		stringToVector3(mainString, position);
 
 		return sectorFactory.createWaypoint(position);
	}
	else
	{
		std::stringstream strStream;
		strStream << "Unknown prefix: " << prefix << " at waypoint definition.";
		throw LoaderException(strStream.str());
	}
}
#endif

#if 0
void Interpreter::interpretRaceCondition(const TagWithAttributesT& definitions, 
                                         RaceCondition& raceCondition, 
                                         std::string& path) const
{
	if (! grab(Tag::path, definitions, path, true))
		return;

	std::string result;
	
	grab(Tag::countdown, definitions, result);
	interpret<f32>(result, raceCondition.mCountdown);
	
	grab(Tag::laps, definitions, result);
	interpret<u32>(result, raceCondition.mLaps);

	grab(Tag::finish, definitions, result);
	interpret<u32>(result, raceCondition.mFinish);

	grab(Tag::timeout, definitions, result);
	interpret<f32>(result, raceCondition.mTimeout);
}
#endif


void Interpreter::interpretLightDefinition(const TagWithAttributesT& definitions,
                                           LightParameters& lightParameters) const
{
	std::string bufferString = "";
	
	grab(Tag::name, definitions, lightParameters.mName);

	grab(Tag::type, definitions, bufferString);
	lightParameters.mLightType = ID::asLightType(bufferString);			

	grab(Tag::direction, definitions, bufferString);
	interpret<v3>(bufferString, lightParameters.mOrientation);
}

void Interpreter::interpretCameraDefinition(const TagWithAttributesT& definitions, 
                                            CameraParameter& cameraParameters, 
                                            std::string& objectName) const
{
	using namespace boost::units;

 	std::string bufferString = "";
 	f32         bufferF32 = 1.0f;

	grab(Tag::type, definitions, bufferString);
	cameraParameters.mMode = ID::asCameraMode(bufferString);

	grab(Tag::position, definitions, objectName);

	grab(Tag::offset, definitions, bufferString);
	interpret<v3>(bufferString, cameraParameters.mOffset);

	if (grab(Tag::fullscreen, definitions, bufferString, true))
	{
	 	if (bufferString == "yes")
	 		cameraParameters.mFullscreen = true;
	 	else
	 		cameraParameters.mFullscreen = false;
	}

	grab(Tag::reactionTime, definitions, bufferString);
	interpret<f32>(bufferString, bufferF32);
	cameraParameters.mReactionTime = bufferF32* si::second;

	grab(Tag::maxDistance, definitions, bufferString);
	interpret<f32>(bufferString, bufferF32);
	cameraParameters.mMaxDistance = bufferF32 * si::meter;

	if (grab(Tag::maxRotationVelocity, definitions, bufferString, true))
		interpret<v3>(bufferString, cameraParameters.mMaxRotationVelocity);

	if (grab(Tag::stiffness, definitions, bufferString, true))
		interpret<f32>(bufferString, bufferF32);
	else
		bufferF32 = 2.8f;
	cameraParameters.mStiffness = bufferF32;
}

void Interpreter::interpretPlaylistDefinition(const TagWithAttributesT& definitions, 
                                              std::string& publicName, 
                                              std::string& filename) const
{
	grab(Tag::name, definitions, publicName);
	grab(Tag::filename, definitions, filename);
}

bool Interpreter::grab(const std::string& tag,
                       const TagWithAttributesT& container,
                       std::string& out,
                       bool optional) const
{
	out.clear();
	
	TagWithAttributesT::const_iterator it = container.find(tag);

	if (it != container.end())
	{
		out = it->second;
		return true;	
	}

	if (optional)
		return false;
	else
		throw LoaderException("Interpreter::grab(): Non-optional tag \"" + tag +
			"\" wasn't found.");
}

template <typename targetT>
void Interpreter::interpret(const std::string& in, targetT& out) const
{
	try
	{
		if (!in.empty())
			convert(in, out);
	}
	catch (std::exception& e)
	{
		std::stringstream ss;
		ss << "Error while trying to cast '" << in
		   << "', at Interpreter::interpret.\n"
		   << "Original exception text: " << e.what();

		throw LoaderException(ss.str());
	}
}

void Interpreter::interpret(const std::string& in, s32& out) const
{
	try
	{
		if (!in.empty())
			convert(in, out);
	}
	catch (std::exception& e)
	{
		//! \note: We could try to use another interpreter at this position.

		// No chance
		std::stringstream ss;
		ss << "Error while trying to cast '" << in
		   << "', at Interpreter::interpret.\n"
		   << "Original exception text: " << e.what();

		throw LoaderException(ss.str());
	}
}

template<typename targetT>
void Interpreter::convert(const std::string& in, targetT& out) const
{
	out = boost::lexical_cast<targetT>(in);
}

void Interpreter::convert(const std::string& in, qv4& out) const
{
	stringToQuaternion4(in, out);
}

void Interpreter::convert(const std::string& in, v3& out) const
{
	stringToVector3(in, out);
}

void Interpreter::convert(const std::string& in, bool& out) const
{
	if      (in == "yes" || in == "true" || in == "1") out = true;
	else if (in == "no" || in == "false" || in == "0") out = false;
}

} // namespace Loader
} // namespace BFG
