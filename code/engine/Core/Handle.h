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

#ifndef BFG_HANDLE_H__
#define BFG_HANDLE_H__

#include <sstream>

#include <boost/assert.hpp>
#include <boost/static_assert.hpp>
#include <boost/integer_traits.hpp>

#include <Core/Enums.hh>
#include <Core/Defs.h>

#define BFG_OWNER_BIT_COUNT 8

namespace BFG
{
	typedef unsigned long long Handle_Type;
	typedef unsigned int ID_Type;
	typedef unsigned int ClassID;
	typedef unsigned int OwnerType;
template <
	size_t   BitsForOwnerId,
	typename H = Handle_Type,
	typename I = ID_Type
>
class BFG_CORE_API HandleTraits
{
public:
	///The raw handle type.
	typedef H Handle_Type; 
	/// The ID
	typedef I ID_Type;     

public: 
    
  /// The handle must be an unsigned integral type with at least 32
  /// bits.
	BOOST_STATIC_ASSERT(  boost::integer_traits<H>::digits >= 32 &&
	                      boost::integer_traits<H>::is_integer   &&
                         !boost::integer_traits<H>::is_signed );

  
	BOOST_STATIC_ASSERT(  boost::integer_traits<I>::is_integer   &&
                         !boost::integer_traits<I>::is_signed    &&
                         (boost::integer_traits<H>::digits >= 2 * boost::integer_traits<I>::digits) );
                        
	/// Number of bits for the dataspace must be less then the number of
	/// bits for an id.
	BOOST_STATIC_ASSERT(  int(BitsForOwnerId) <
                          boost::integer_traits<H>::digits - boost::integer_traits<I>::digits );
  
public: 
  /// number of bits for the handle.
  static const size_t bits = sizeof(H)*8; // 4 bytes * 8 bit/bytes

  /// number of bits for an object id.
  static const size_t bitsForId = boost::integer_traits<I>::digits;

  static const size_t bitsForOwner = BitsForOwnerId;

  static const size_t bitsForCID   = (bits-bitsForOwner-bitsForId-1);
  /// number of bits must not overwhelm available bits.
  BOOST_STATIC_ASSERT( bitsForId+BitsForOwnerId <= bits );

public: 
  /// First bit is for Ids.
  static const Handle_Type startId   = H(0);

  /// Mask for Ids.
  static const Handle_Type maskId   = ((H(1)<<bitsForId)-H(1));

  static const Handle_Type startOwnerID = bitsForId;
  
  static const Handle_Type maskOwnerID = ((H(1)<<bitsForOwner)-H(1)) << startOwnerID;

  /// First bit for class ids.
  static const Handle_Type startCID     = bitsForId+bitsForOwner;

  /// Mask for class ids.
  static const Handle_Type maskCID     = ((H(1)<<bitsForCID)-H(1)) << startCID;

  /// The invalid bits.
  static const Handle_Type InvalidBit = H(1) << (bits-1);

  
};

// The Handle itself

class BFG_CORE_API Handle : public HandleTraits<BFG_OWNER_BIT_COUNT>
{
public: /// Maximum values ???

  static const size_t max_IDs;
  static const size_t max_Owners;
  static const size_t max_Classes;

  //typedef HandleTraits::Handle_Type Handle_Type;
public:

  /// global handle.
  static Handle_Type globalObject(ClassID cid, ID_Type id)
  {
    return makeHandle(cid, 0, id);
  }

  /// normal handle
  static Handle_Type normalObject(ClassID cid, OwnerType owner, ID_Type id)
  {
    return makeHandle(cid, owner, id);
  }

  /// singleton
  static Handle_Type singletonObject(ClassID cid, OwnerType owner)
  {
    return makeHandle(cid, owner, 0);
  }

  /// Invalid object handle
  static Handle_Type invalidHandle(ClassID cid, OwnerType owner, ID_Type id=0)
  {
    return validate(makeHandle(cid, owner, id),false);
  }

public:

  static bool isInvalid( Handle_Type handle )
  {
    return (handle & InvalidBit) != 0;
  }

  static bool isValid( Handle_Type handle )
  {
    return (handle & InvalidBit) == 0;
  }
 
public: 
  /// extract class id from handle.
  static ClassID CID(Handle_Type handle)
  {
    return ClassID((handle & maskCID) >> startCID);
  }

  /// extract owner id from handle.
  static OwnerType Owner( Handle_Type handle)
  {
    return OwnerType( (handle & maskOwnerID) >> startOwnerID );
  }

  /// extract object id from handle.
  static ID_Type ID(Handle_Type handle)
  {
    return ID_Type( (handle & maskId) >> startId );
  }

  //@}

public: /// \name Manipulators
  //@{

 /* static handle_type replaceIdObj( const handle_type _h, const id_type _id_obj )
  {
    BOOST_ASSERT(isValid(_h));
    BOOST_ASSERT(_id_obj < max_idObj);
    return (_h & ~mIdObj)|(handle_type(_id_obj) << sIdObj);
  }

  static handle_type replaceCid( const handle_type _h, const ECID _cid )
  {
    BOOST_ASSERT(isValid(_h));
    return (_h & ~mCID)|(_cid << sCID);
  }
  */

  static Handle_Type invalidate( Handle_Type& handle )
  {
    return handle|=InvalidBit;
  }

  static Handle_Type validate( const Handle_Type& handle, bool isValid )
  {
    return isValid ? handle : (handle|InvalidBit);
  }

  static Handle_Type makeValid(const Handle_Type& handle)
  {
	  return (isValid(handle) ? handle : (handle ^ InvalidBit));
  }

  //@}
protected:

  static Handle_Type makeHandle(ClassID cid, OwnerType owner, ID_Type id)
  {
    /*BOOST_ASSERT( owner  < max_Owner );
    BOOST_ASSERT( id < max_ID );
    BOOST_ASSERT( cid   != CID_INVALID );*/
    Handle_Type handle = (((Handle_Type)cid) << startCID)|(Handle_Type(owner) << startOwnerID)|Handle_Type(id);
    return validate( handle , cid != ID::CID_INVALID );
  }

};


const std::string toString(const Handle_Type& handle)
{
  std::stringstream str;
  str << "(" << (Handle::isValid(handle) ? "v" : "i" )
     << "|"  << Handle::CID(handle)
     << ":"  << Handle::Owner(handle)
     << ":"  << Handle::ID(handle) 
     << ")";
  return str.str();
}  


} //namespace BFG
#endif //__BFG_HANDLE_H__
