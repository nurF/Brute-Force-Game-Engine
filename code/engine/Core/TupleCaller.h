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

#ifndef BFG_TUPLE_CALLER_
#define BFG_TUPLE_CALLER_

#include <boost/tuple/tuple.hpp>
#include <boost/fusion/tuple.hpp>
#include <boost/preprocessor/repetition.hpp>
#include <boost/function_types/result_type.hpp>
#include <boost/utility/enable_if.hpp>

namespace BFG
{

namespace detail
{

template< int N >
struct tupleCaller;

#ifndef MAX_TUPLE_ARGUMENT_COUNT
	//! default value is 15
	#define MAX_TUPLE_ARGUMENT_COUNT 15
#endif

//! macro is used by #DEF_TUPLE_CALL_FUN
#define ARG_FUN(N, I, _) BOOST_PP_COMMA_IF(I) get< I >(t)

//! macro is used by #BOOST_PP_REPEAT_2ND()
#define DEF_TUPLE_CALL_FUN(N, I, _) \
template<> struct tupleCaller< I > \
{ \
	template<typename ReturnT BOOST_PP_COMMA() typename CallB BOOST_PP_COMMA() typename TupleT> \
	static ReturnT call(CallB c BOOST_PP_COMMA() const TupleT &t) \
	{ \
		using boost::get; \
		return c(BOOST_PP_REPEAT(I, ARG_FUN, _) ); \
	}\
	template<typename T, typename TupleT> \
	static T* create(const TupleT &t) \
	{ \
		using boost::get; \
		return new T(BOOST_PP_REPEAT(I, ARG_FUN, _) ); \
	} \
};

/** \brief Will generate MAX_TUPLE_ARGUMENT_COUNT template specializations wich are used by callTuple(CallB , TupleT).

The macro will generate specializations of tupleCaller for boost::tuple's with up to MAX_TUPLE_ARGUMENT_COUNT
elements. In the following example the tuple has a length of <i>N</i>
Example:
\code
template<>
struct tupleCaller<N>
{
	template<typename ReturnT, typename CallB, typename TupleT>
	static ReturnT call(CallB c , const TupleT &t)
	{
		using boost::get;
		c(get<0>(t), get<1>(t), ..., get<N-1>(t));
	}
};
\endcode
*/
BOOST_PP_REPEAT_2ND
( BOOST_PP_INC(MAX_TUPLE_ARGUMENT_COUNT)
 , DEF_TUPLE_CALL_FUN
 , _
 )

#undef ARG_FUN
#undef DEF_TUPLE_CALL_FUN

//!helper struct to determine the size of an boost::tuple<...> or an boost::fusion::tuple<...>
template<typename T, typename Enable = void>
struct TupleSizeHelper
{ };

template<typename T>
struct TupleSizeHelper<T, typename boost::enable_if< boost::fusion::traits::is_sequence<T> >::type >
{
	enum
	{
		length = boost::fusion::tuple_size<T>::value
	};
};

template<typename T>
struct TupleSizeHelper<T, typename boost::disable_if< boost::fusion::traits::is_sequence<T> >::type >
{
	enum
	{
		length = boost::tuples::length<T>::value
	};
};


} // namespace detail

/** \brief Will call the function given in c and pass all elements in the tuple as parameters

\tparam TupleT Musst be of type boost::fusion::tuple<...> or boost::tuple
\param c Musst be sth. callable that accepts as much parameters an the given tuple has elements (of course the types muss also match)
\return Return type is equal to the return type of the given function c
*/
template<typename TupleT, typename CallB>
typename boost::function_types::result_type<CallB>::type
callTuple(CallB c, const TupleT& t)
{
	typedef typename boost::function_types::result_type<CallB>::type ReturnType;
	return detail::tupleCaller< detail::TupleSizeHelper<TupleT>::length >::call<ReturnType>(c, t);
}

/** \brief Will call the new operator on the given type T and pass all elements in the given tuple to the constructor of T

\tparam T The new operator will be called on the type. It musst be given separately as template parameter.
\tparam TupleT Musst be of type boost::fusion::tuple<...> or boost::tuple
\return Pointer to the created Object.
*/
template<typename T, typename TupleT>
T* constructFromTuple(TupleT t)
{
 	return detail::tupleCaller< boost::fusion::tuple_size<TupleT>::value >::create<T>(t);
}

} // namespace BFG

#endif