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

#ifndef BFG_PROXY_POLICY_H_
#define BFG_PROXY_POLICY_H_


#include <Core/ProxyStorage.h>
#include <boost/mpl/bool.hpp>

namespace BFG
{

template< typename Type, template<typename> class StorageT >
class rwPolicy : public StorageT< Type >
{
public:
    typedef StorageT< Type > StorageType;
    typedef boost::mpl::bool_<false> IsConst;
    static bool isConst() { return false; }

public:
    rwPolicy() : StorageType ()
    {
    
    }

    rwPolicy( const StorageType& rhs ) : StorageType (rhs)
    {
    
    }

public:
    const Type& ref() const { return this->get_ref(); }
    Type& ref() { return this->get_ref(); }
    
    const Type* operator -> () const { return this->get_ptr(); }
    Type* operator -> () { return this->get_ptr(); }
    
    const Type& operator * () const { return this->get_ref(); }
    Type& operator * () { return this->get_ref(); }
};

template < typename Type, template<typename> class StorageT >
class roPolicy : public StorageT < Type >
{
public:    
    typedef StorageT< Type > StorageType;
    typedef boost::mpl::bool_<true> IsConst;
    static bool isConst() { return true; }
    
public:
    roPolicy() : StorageType ()
    {
    
    }
    
    roPolicy( const StorageType& rhs ) : StorageType (rhs)
    {
    
    }

    roPolicy( const rwPolicy< Type, StorageT >& rhs ) : StorageType (rhs)
    {
    
    }
    
public:    
    roPolicy& operator = ( const rwPolicy< Type, StorageT >& rhs )
    {
        return (*this = roPolicy< Type, StorageT >(rhs));    
    }
    
    const Type& ref() const { return this->get_ref(); }
    const Type* operator -> () const { return this->get_ptr(); }
    const Type& operator *  () const { return this->get_ref(); }
    
};

} //namespace BFG

#endif // BFG_PROXY_POLICY_H_
