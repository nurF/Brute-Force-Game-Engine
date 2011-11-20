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

#ifndef LOADER_TYPES_H_
#define LOADER_TYPES_H_

#include <string>
#include <vector>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>

#include <Core/Location.h>
#include <Core/Types.h>
#include <Model/Property/Value.h>
#include <Model/Property/ValueId.h>
#include <Model/Property/ConceptId.h>
#include <Model/Enums.hh>
#include <Physics/Enums.hh>
#include <View/Enums.hh>

/**
	This file contains all object types and data-structures that are needed
	for the loading/interpreting/creation/building process.
	
	\todo
	Some parts of the (for instance camera) uses their own data-structures which 
    are defined in their own files. This structures have to be moved to this file.
*/

namespace BFG {

class PropertyConcept;

namespace Loader {

/*! These three following types are crucial to save the native string data
    ready to get interpreted by the interpreter (Interpreter.h)*/
typedef boost::unordered_map
<
	std::string,
	std::string
> TagWithAttributesT;

//! \note The actual tag name is not needed here (see ManyManyTagsT)
typedef std::vector
<
	boost::shared_ptr<TagWithAttributesT>
> ManyTagsT;

//! \note The key type is actually the name attribute of a ManyTagsT
typedef boost::unordered_map
<
	std::string,
	boost::shared_ptr<ManyTagsT>
> ManyManyTagsT;

//! This struct will be filled by the file loader (FileLoader.h) to save the native string data of one sector (level)
struct SectorDraft
{
	boost::shared_ptr<ManyTagsT>          mObjects;
	boost::shared_ptr<ManyManyTagsT>      mPaths;
	boost::shared_ptr<TagWithAttributesT> mSectorProperties;
	boost::shared_ptr<TagWithAttributesT> mConditions;
	boost::shared_ptr<ManyTagsT>          mCameras;
	boost::shared_ptr<TagWithAttributesT> mSkybox;
	boost::shared_ptr<ManyTagsT>          mLights;
	boost::shared_ptr<ManyTagsT>          mPlaylist;
};

struct Connection
{
	Connection() : mConnectedLocalAt(0), mConnectedExternAt(0) {}

	bool good() const
	{
		return ! mConnectedExternToModule.empty() &&
			mConnectedExternAt > 0 &&
			mConnectedLocalAt > 0;
	}

	u32 mConnectedLocalAt;
	std::string mConnectedExternToGameObject;
	std::string mConnectedExternToModule;
	u32 mConnectedExternAt;
};

struct ValueParameter
{
	Property::ValueId mId;
	Property::Value mValue;
};

struct ConceptParameter
{
	Property::ConceptId mId;
	std::string mProperties;	
};

struct AdapterParameter
{
	u32 mId;
	Location mLocation;
};

struct ModuleParameter
{
	ModuleParameter() : 
		mCollisionMode(ID::CM_Disabled),
		mVisible(false),
		mDensity(0) {}

	std::string mName;
	std::string mMeshName;
	Connection mConnection;
	std::string mAdapters;
	std::string mConcepts;
	ID::CollisionMode mCollisionMode;
	bool mVisible;
	f32 mDensity;
};

//! This struct saves the interpreted object data which is not(!) defined by PropertyConcepts.
struct ObjectParameter
{
	ObjectParameter() :
		mHandle(NULL_HANDLE),
		mLinearVelocity(v3::ZERO),
		mAngularVelocity(v3::ZERO) {}

	GameHandle mHandle;
	std::string mName;
	std::string mType;
	Location mLocation;
	v3 mLinearVelocity;
	v3 mAngularVelocity;
	Connection mConnection;
};

struct LightParameters
{
	LightParameters() : mOrientation(v3::ZERO) {}	

	std::string   mName;
	ID::LightType mLightType;
	v3            mOrientation;
};

//! Contains the complete data for all objectTypes ready for creation.
typedef std::vector<boost::shared_ptr<ObjectParameter> > ObjectDefinitionsT;

struct LevelConfig
{
	typedef std::vector<std::string> ConfigFilesT;
	
	ConfigFilesT mModules;
	ConfigFilesT mAdapters;
	ConfigFilesT mConcepts;
	ConfigFilesT mProperties;
};

} // namespace Loader
} // namespace BFG

#endif
