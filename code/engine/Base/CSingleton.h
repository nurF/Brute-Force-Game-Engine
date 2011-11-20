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

#ifndef __C_SINGLETON_H_
#define __C_SINGLETON_H_

#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include "CDummyMutex.h"

namespace BFG {
namespace Base {


class CNoInstantiation
{
protected:
  
  template <class UserClassT>
  static void create( UserClassT*& ) 
  {  
    // do nothing
  }

};

/// \brief :TODO: Explain functionality
class CLazyObject
{
protected:

  /**
   * :TODO: explain concept of Lazy-Instantiation 
   * and implement other Policy-Types
   *
   * Need to be friend with UserClassT
   */
  template <class UserClassT>
  static void create( UserClassT*& _ptr ) 
  {  
    _ptr = new UserClassT;
	static boost::scoped_ptr<UserClassT> destroyer(_ptr);  
  }
};

/// \brief The new BFG Singleton pattern.
///
/// Based on Andrei Alexandrescu's idea from the book "Modern C++ Design" 
///
/// Normally the Lock is CDummyMutex.
/// If you have problems using the Singleton
/// friend class 
///
template < class UserClassT, 
		   class Lock=CDummyMutex,
           class InstancePolicy=CLazyObject
         >
class CSingletonT : private InstancePolicy, private boost::noncopyable
{
public:

  typedef InstancePolicy InstantiationPolicyT;
  typedef Lock            LockT;

  /// \brief Access to instance.
  ///
  /// Provide access to the single instance through double-checked locking 
  ///
  /// \return T* single instance 
  ///
  static UserClassT* instance()
  {
    // Uses local static storage to avoid static construction
    // sequence issues. (regaring when the lock is created)
    static UserClassT* ptr = 0;    
    static LockT lock;
    
    if(!ptr) 
    {
      lock.lock();
      if(!ptr)        
        InstancePolicy::create(ptr);
	  lock.unlock();
    }   
    return const_cast<UserClassT*>(ptr);    
  }
};

// Of course you can manually add the Instance-Function to your class
// These two macros are for the lazy programers
#define DECLARE_SINGLETON(X) static X* instance();
#define DEFINE_SINGLETON(X) X* X::instance() { return Base::CSingletonT<X>::instance(); }
//#define SINGLETON_ACCESS_CTOR friend class Base::CLazyObject;

} // namespace Base
} // namespace BFG

#endif //__C_SINGLETON_H_
