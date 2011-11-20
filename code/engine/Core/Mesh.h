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

#ifndef BFG_MESH_H
#define BFG_MESH_H

#include <boost/scoped_array.hpp>
#include <Core/Types.h>
#include <Core/v3.h>
#include <Core/Defs.h>

namespace BFG {

struct BFG_CORE_API Mesh
{
	Mesh();

	Mesh(v3* mVertices,
	     u32 mVertexCount,
	     u32* mIndices,
	     u32 mIndexCount);
	     
	Mesh(const Mesh& mesh);

	Mesh& operator = (Mesh rhs);
	
	~Mesh();

	boost::scoped_array<v3> mVertices;
	u32                     mVertexCount;

	boost::scoped_array<u32> mIndices;
	u32                      mIndexCount;
	
private:
	friend void swap(Mesh& first, Mesh& second)
	{
		using std::swap; 

		swap(first.mVertices, second.mVertices); 
		swap(first.mVertexCount, second.mVertexCount); 

		swap(first.mIndices, second.mIndices);
		swap(first.mIndexCount, second.mIndexCount);
	}

	void take_copy(v3* vertices,
	               u32 vertexCount,
	               u32* indices,
	               u32 indexCount);
};

} // namespace BFG

#endif
