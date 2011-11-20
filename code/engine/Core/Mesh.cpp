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

#include <Core/Mesh.h>

#include <cstring>

namespace BFG {

Mesh::Mesh() :
mVertexCount(0),
mIndexCount(0)
{
}

Mesh::Mesh(v3* vertices,
	       u32 vertexCount,
	       u32* indices,
	       u32 indexCount)
{
	take_copy
	(
		vertices,
		vertexCount,
		indices,
		indexCount
	);
}

Mesh::Mesh(const Mesh& mesh)
{
	take_copy
	(
		mesh.mVertices.get(),
		mesh.mVertexCount,
		mesh.mIndices.get(),
		mesh.mIndexCount
	);
}

Mesh::~Mesh()
{
}

Mesh& Mesh::operator = (Mesh rhs)
{
	swap(*this, rhs);
	return *this;
}

void Mesh::take_copy(v3* vertices,
	                 u32 vertexCount,
	                 u32* indices,
	                 u32 indexCount)
{
	mVertexCount = vertexCount;
	mIndexCount = indexCount;

	mVertices.reset(new v3[vertexCount]);
	mIndices.reset(new u32[indexCount]);
	
	memcpy(mVertices.get(), vertices, vertexCount*sizeof(v3));
	memcpy(mIndices.get(), indices, indexCount*sizeof(u32));		
}


} // namespace BFG
