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

#ifndef BFG_PROXY_STORAGE_H_
#define BFG_PROXY_STORAGE_H_

#include <boost/shared_ptr.hpp>
#include <boost/type_traits.hpp>
#include <boost/mpl/bool.hpp>

namespace BFG
{

template <typename Type>
class InheritStorageT : protected Type
{
public:
    typedef boost::mpl::bool_<true>  HasBase;
    typedef boost::mpl::bool_<true>  IsObject;
    typedef boost::mpl::bool_<false> IsReference;   
    
    typedef InheritStorageT<Type>    ThisType;
public:
    // \name ctors
    // @{
    InheritStorageT() : Type()
    {
    
    }
    
    InheritStorageT( const InheritStorageT& rhs ) : Type (rhs)
    {
    
    }
    
    template< template < typename >  class S >
    InheritStorageT( const S< Type >& rhs ) : Type( rhs.ref() )
    {
    
    }
    
    template< typename T2 >
    InheritStorageT( const InheritStorageT< T2 >& rhs ) : Type()
    {
        if( boost::is_base_and_derived< Type, T2 >::value )
        {
            // in this case the user has to supply a proper init function for T
            Type::init(rhs);
        }
        else
        {
            throw(std::logic_error("Tried to call copy constructor with incompatible type"));
        }
    }
    // @}
protected:
    const Type& get_ref() const { return *this; }
    Type& get_ref() { return *this; }
    
    const Type* get_ptr() const { return this; }
    Type* get_ptr () { return this; }
    
    bool isValid() const { return true; }
};


template <typename Type> 
class PointerStorageT
{
public:
    typedef typename boost::shared_ptr<Type> pointer_type;

    typedef boost::mpl::bool_<false>  HasBase;
    typedef boost::mpl::bool_<false>  IsObject;
    typedef boost::mpl::bool_<true> IsReference;   

public:
    // \name ctors
    // @{
    PointerStorageT()
    {
    
    }

    PointerStorageT( pointer_type _ptr ) : ptr(_ptr)
    {
    
    }

    PointerStorageT( const PointerStorageT& rhs ) : ptr (rhs.ptr)
    {
        
    }

    template< template <typename> class S>
    PointerStorageT( const S<Type>& rhs ) : ptr ( new pointer_type(new Type(rhs.ref())))
    {

    }
    
    template< typename T2 >
    PointerStorageT( const PointerStorageT< T2 >& rhs )
    {
        if( boost::is_base_and_derived<Type, T2>::value )
        {
            ptr = rhs.pointer();      
        }
        else
        {
            throw(std::logic_error("Tried to call copy constructor with incompatible type"));
        }   
    }
    // @}
public:
    bool    isNull()    const { return ptr.get() == 0 ; }
    size_t  use_count() const { return ptr.use_count(); }
    
protected:    
    // interface
    pointer_type pointer() const { return ptr; }
    bool isStorageValid() const { return !isNull(); }
    
    const Type& get_ref() const { return *ptr; }
    Type& get_ref() { return *ptr; }
    
    const Type* get_ptr() const { return ptr.get(); }
    Type* get_ptr() { return ptr.get(); }
    
private:
    pointer_type ptr;    
};

}

#endif // BFG_PROXY_STORAGE_H_
