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

#ifndef VIP_ENUM_TO_TYPE_H
#define VIP_ENUM_TO_TYPE_H

#include <boost/mpl/vector.hpp>
#include <boost/mpl/find_if.hpp>
#include <boost/mpl/equal_to.hpp>
#include <boost/mpl/long.hpp>
#include <boost/mpl/deref.hpp>

#include <Controller/VIPGenerator.h>

namespace BFG {
namespace Controller_ {
namespace VIP {

namespace mpl = boost::mpl;

//! Template magic for struct Get<>
namespace detail {

template <int id>
struct IdT
{
	template <typename T>
	struct apply
	{
		typedef mpl::long_<
			id
		> id_type;
		
		typedef mpl::long_<
			T::ID
		> t_type;
				
		typedef mpl::equal_to <
			t_type,
			id_type
		> type;
	};
};

template <int id>
struct Get_
{
	typedef ALL_TYPES AllVips;
	
	typedef typename
	mpl::find_if <
		AllVips,
		IdT<id>
	>::type Type;
};

} // namespace detail

//! Returns the real class type of a corresponding enum identifier.
//! Requirement is an anonymous enum member `ID' and that the VIP type is
//! member of the type vector `ALL_TYPES' in VIPGenerator.h
template <int id>
struct Get
{
	typedef typename mpl::deref <
		typename detail::Get_<id>::Type
	>::type Type;
	
	typedef typename Type::EnvT Env;
	
	template <typename InitializerT>
	static VipPtrT makePtr(InitializerT& initializer)
	{
		Env env;
		initializer.Init(env);
		VipPtrT vip(new Type(env));
		return vip;
	}
};

template <typename InitializerT>
VipPtrT createByEnum(InitializerT& initializer, ID::VIPType vt)
{

}

} // namespace VIP
} // namespace Controller_
} // namespace BFG

#endif
