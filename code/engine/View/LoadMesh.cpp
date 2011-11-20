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
#include <View/LoadMesh.h>

#include <stdexcept>

#include <OgreDefaultHardwareBufferManager.h>
#include <OgreMeshManager.h>
#include <OgreResourceGroupManager.h>
#include <OgreRoot.h>
#include <OgreSubMesh.h>

#include <Core/v3.h>
#include <Core/qv4.h>
#include <Core/Path.h>

namespace BFG {
namespace View {

void initOgreForMeshLoading()
{
	Path p;
	const std::string meshPath = p.Get(ID::P_GRAPHICS_MESHES);

	Ogre::Root* root = new Ogre::Root("", "", p.Get(ID::P_LOGS) + "OgreMeshLoad.log");
	new Ogre::DefaultHardwareBufferManager();
	Ogre::ResourceGroupManager::getSingleton().addResourceLocation(meshPath, "FileSystem");
	Ogre::ResourceGroupManager::getSingleton().initialiseAllResourceGroups();	
}

Mesh loadMesh(const std::string& meshName)
{
	Mesh result;

	bool addedShared = false;
	size_t currentOffset = 0;
	size_t sharedOffset = 0;
	size_t nextOffset = 0;
	size_t indexOffset = 0;

	const v3 scale        = v3::UNIT_SCALE;
	const v3 position     = v3::ZERO;
	const qv4 orientation = qv4::IDENTITY;

	Ogre::MeshManager* mm = Ogre::MeshManager::getSingletonPtr();
	
	if (! mm)
	{
		initOgreForMeshLoading();
		mm = Ogre::MeshManager::getSingletonPtr();
	}

	Ogre::MeshPtr mesh = mm->load(meshName, "General");

	// Calculate how many result.mVertices and result.mIndices we're going to need
	for (unsigned short i=0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* subMesh = mesh->getSubMesh(i);

		// We only need to add the shared result.mVertices once
		if (subMesh->useSharedVertices)
		{
			if (!addedShared)
			{
				result.mVertexCount += mesh->sharedVertexData->vertexCount;
				addedShared = true;
			}
		}
		else
		{
			result.mVertexCount += subMesh->vertexData->vertexCount;
		}

		result.mIndexCount += subMesh->indexData->indexCount;
	}

	result.mVertices.reset(new v3[result.mVertexCount]);
	result.mIndices.reset(new u32[result.mIndexCount]);

	addedShared = false;

	// Run through the sub-meshes again, adding the data into the arrays
	for (unsigned short i=0; i < mesh->getNumSubMeshes(); ++i)
	{
		Ogre::SubMesh* subMesh = mesh->getSubMesh(i);

		Ogre::VertexData* vertexData = subMesh->useSharedVertices ? mesh->sharedVertexData : subMesh->vertexData;

		if((!subMesh->useSharedVertices) || (subMesh->useSharedVertices && !addedShared))
		{
			if(subMesh->useSharedVertices)
			{
				addedShared = true;
				sharedOffset = currentOffset;
			}

			const Ogre::VertexElement* posElem = 
			    vertexData->vertexDeclaration->findElementBySemantic(Ogre::VES_POSITION);

			Ogre::HardwareVertexBufferSharedPtr vbuf = 
			    vertexData->vertexBufferBinding->getBuffer(posElem->getSource());

			unsigned char* vertex =	static_cast<unsigned char*>(vbuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));

			// There is _no_ baseVertexPointerToElement() which takes an Ogre::Real or a double
			// as second argument. So make it float, to avoid trouble when Ogre::Real will
			// be compiled/typedefed as double:

			float* pReal;

			for( size_t j = 0; j < vertexData->vertexCount; ++j, vertex += vbuf->getVertexSize())
			{
				posElem->baseVertexPointerToElement(vertex, &pReal);
				v3 pt(pReal[0], pReal[1], pReal[2]);
				result.mVertices[currentOffset + j] = (orientation * (pt * scale)) + position;
			}

			vbuf->unlock();
			nextOffset += vertexData->vertexCount;
		}

		Ogre::IndexData* index_data = subMesh->indexData;
		size_t numTris = index_data->indexCount / 3;
		Ogre::HardwareIndexBufferSharedPtr ibuf = index_data->indexBuffer;

		bool use32bitindexes = (ibuf->getType() == Ogre::HardwareIndexBuffer::IT_32BIT);

		unsigned long*  pLong = static_cast<unsigned long*>(ibuf->lock(Ogre::HardwareBuffer::HBL_READ_ONLY));
		unsigned short* pShort = reinterpret_cast<unsigned short*>(pLong);


		size_t offset = (subMesh->useSharedVertices)? sharedOffset : currentOffset;

		if (use32bitindexes)
		{
			for (size_t k = 0; k < numTris*3; ++k)
			{
				result.mIndices[indexOffset++] = pLong[k] + static_cast<unsigned long>(offset);
			}
		}
		else
		{
			for (size_t k = 0; k < numTris*3; ++k)
			{
				result.mIndices[indexOffset++] = static_cast<unsigned long>(pShort[k]) + static_cast<unsigned long>(offset);
			}
		}

		ibuf->unlock();
		currentOffset = nextOffset;
	}
	
	return result;
}

} // namespace View
} // namespace BFG
