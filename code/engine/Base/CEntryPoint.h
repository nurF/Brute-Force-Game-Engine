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

#ifndef __BFG_BASE_C_ENTRY_POINT_H_
#define __BFG_BASE_C_ENTRY_POINT_H_

#include <string>
#include <vector>
#include <boost/scoped_ptr.hpp>
#include <Base/Compiler.h>

namespace BFG {
namespace Base {

typedef void *(*EntryPointCallbackT)(void*);

class IEntryPoint
{
public:
	IEntryPoint(const std::string& desc) :
	description(desc)
	{}
	
	virtual ~IEntryPoint()
	{}

	virtual void* Run (void* data) = 0;
	virtual void* Stop(void* data) = 0;
	
	const std::string& Description() const { return description; }
	
private:
	std::string description;
};

template <class EntryPointT = EntryPointCallbackT>
class CEntryPointBase : public IEntryPoint
{
public:
	//! Our function type for entrypoints,
	//! think about boost::function for the future.
	CEntryPointBase(EntryPointT start, EntryPointT stop, const std::string& desc) :
	IEntryPoint(desc),
	startEntry(start),
	stopEntry(stop)
	{}

	virtual ~CEntryPointBase()
	{}

protected:
	EntryPointT startEntry;
	EntryPointT stopEntry;
};

typedef void *(*StaticEntryPointT)(void*);

class CEntryPoint : public CEntryPointBase<StaticEntryPointT>
{
public:
	CEntryPoint(StaticEntryPointT start, StaticEntryPointT stop = (StaticEntryPointT)NULL, const std::string& description = std::string("")):
	CEntryPointBase<StaticEntryPointT>(start, stop, description)
	{}

	virtual ~CEntryPoint()
	{}

	virtual void* Run(void* data)
	{
		return startEntry(data);
	}
	
	virtual void* Stop(void* data)
	{
	    if ( stopEntry )
	        return stopEntry(data);
	    else
	        return NULL;
	}
};


template <class Class>
class CClassEntryPoint : public CEntryPointBase < void *(Class::*)(void*)>
{
public:
	using CEntryPointBase < void *(Class::*)(void*)>::stopEntry;

	//! \param p If supplied, this class will take ownership of it
	CClassEntryPoint(Class* p, 
	                 void*(Class::*start)(void*), 
	                 void*(Class::*stop)(void*) = NULL, 
	                 const std::string& description=std::string("")) :
	CEntryPointBase<void*(Class::*)(void*)> (start, stop, description),
	mCallbackObject(p)
	{
		assert(p && "CClassEntryPoint: Callback object pointer must not be zero!");
	}

	virtual void* Run(void* data) 
	{
		
		return (*mCallbackObject.*(CEntryPointBase<void*(Class::*)(void*)>::startEntry))(data);
	}

    virtual void* Stop(void* data)
    {
        if(stopEntry)
            return (*mCallbackObject.*(CEntryPointBase<void*(Class::*)(void*)>::stopEntry))(data);
        else
           return NULL;
    }

private:  
	boost::scoped_ptr<Class> mCallbackObject;
};

template <class ClassType>
class CEntryPointHandler 
{
public:
	void callEntryPoints(ClassType* that)
	{ 
		for ( size_t i = 0; i < mEntryPoints.size(); ++i )
		{
			// Call EntryPoint
			mEntryPoints[i]->Run(that);
		}
	}

	bool hasEntryPoints()
	{
		return mEntryPoints.size() > 0;
	}

	//! Setting the EntryPoint for the EventLoop, should consider multiple EntryPoints for a Single EventLoop
	void addEntryPoint(BFG::Base::IEntryPoint* start_routine)
	{
		mEntryPoints.push_back(start_routine);
	}

private:
	//! The EntryPoints for this EventLoop
	std::vector<BFG::Base::IEntryPoint*> mEntryPoints;
};

} // namespace Base
} // namespace BFG

#endif // __BFG_BASE_C_ENTRY_POINT_H_
