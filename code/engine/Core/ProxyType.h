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

#ifndef BFG_PROXY_TYPE_H_
#define BFG_PROXY_TYPE_H_

// Includes ------------------------------------------------------------------
// self
#include <Core/ProxyStorage.h>
#include <Core/ProxyPolicy.h>

namespace BFG
{

template <typename _T> struct TypeWrapper
{
    typedef TypeWrapper<_T> type;
};

#ifdef _MSC_VER
template < template <typename, template<typename> class > class _T2 > struct TypeWrapper2
{
    typedef typename TypeWrapper2< _T2 > type;
};
#endif

template<   class Type, 
            template <typename, template<typename> class > class accessPolicy,
            template <typename> class StorageType
        >
class ProxyType : public accessPolicy < Type, StorageType >
{
public:
    typedef Type DataT;
#ifdef _MSC_VER 
    typename TypeWrapper < StorageType  >::type StorageT;
    typename TypeWrapper2< accessPolicy >::type AccessT;
    
#else    
    typedef StorageType  < Type >                           StorageT;
    typedef accessPolicy< Type, StorageType >              AccessT;
    typedef ProxyType< Type, accessPolicy, StorageType >    ProxyT;
#endif //_MSC_VER
    // \name ctors
    // @{
    // \brief default constructor
    ProxyType() : AccessT()
    {
    
    }
    // \brief copy constructor
    ProxyType( const ProxyType& rhs ) : AccessT( rhs )
    {
    
    }
    
    // \brief different storage copy constructor
    template< template <typename> class S >
    ProxyType( const ProxyType< Type, accessPolicy, S >& rhs ) : AccessT(rhs)
    {
    
    }
    // @}
    template< template <typename> class S >
    ProxyType( const rwPolicy< DataT, S >& rhs ) : AccessT(rhs)
    {
    
    }
    
    bool isValid() const { return this->isStorageValid() && this->ref(); }
};

} // namespace BFG

#endif // BFG_PROXY_TYPE_H_
