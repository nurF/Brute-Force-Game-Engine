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

#ifndef PROPERTYVALUEID_H_
#define PROPERTYVALUEID_H_

#include <ostream>
#include <boost/static_assert.hpp>
#include <Core/Types.h>

namespace BFG {
namespace Property {

template <typename PartialT_, typename CompleteT_>
struct ComposedId
{
	typedef PartialT_                            PartialT;
	typedef CompleteT_                           CompleteT;
	
	typedef PartialT                             PluginIdT;
	typedef PartialT                             VarIdT;
	
	typedef ComposedId<PartialT, CompleteT>      ThisT;
	
	const static PluginIdT ENGINE_PLUGIN_ID;
	
	const static size_t PARTIAL_TYPE_BITS = sizeof(PartialT) * 8;

	ComposedId() :
	mVarId(0),
	mPluginId(0)
	{
	}

	ComposedId(PartialT vid, PartialT pid) :
	mVarId(vid),
	mPluginId(pid)
	{
		// One 'CompleteT' consists out of exactly two 'PartialT's.
		BOOST_STATIC_ASSERT((sizeof(PartialT)*2 == sizeof(CompleteT)));
	}
	
	CompleteT full() const
	{
		return static_cast<CompleteT>(mPluginId) << PARTIAL_TYPE_BITS | mVarId;
	}

	bool operator < (const ThisT& rhs) const
	{
		return this->full() < rhs.full();
	}

	friend std::ostream& operator << (std::ostream& lhs,
	                                  const ThisT& rhs)
	{
		lhs << "("
		    << "VID: " << rhs.mVarId << ", "
		    << "PID: " << rhs.mPluginId
		    << ")";
		return lhs;
	}
	
	VarIdT    mVarId;
	PluginIdT mPluginId;
};

// Static template class member definition
template <typename PartialT_, typename CompleteT_>
const PartialT_ ComposedId<PartialT_, CompleteT_>::ENGINE_PLUGIN_ID = 0;

// These should be the most common cases
typedef ComposedId<s16, s32> ValueId;
typedef ValueId::PluginIdT PluginId;

namespace detail
{
	static size_t g_Plugins = 0;
}

template <typename PartialT_>
PartialT_ generatePluginId()
{
	return ++detail::g_Plugins;
}

} // namespace Property
} // namespace BFG

#endif
