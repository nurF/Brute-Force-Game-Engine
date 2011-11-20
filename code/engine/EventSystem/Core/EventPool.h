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

#ifndef __EVENT_POOL_H_
#define __EVENT_POOL_H_

#ifdef USE_SSE2
#include <emmintrin.h>
#endif

#include <cstdlib>
#include <cstring>
#include <stdexcept>

#include <boost/serialization/split_member.hpp>
#include <boost/serialization/binary_object.hpp>

#include <EventSystem/Core/EventDefs.h>


static const size_t PoolMaxEvents = 40960;

static const size_t AlignmentValue = 16;


template <class Type>
class EventPool
{
	typedef Type* TypePtr;
	typedef EventPool<Type> MyType;
	typedef int* InternalPointerType;

public:
	EventPool(size_t listSize = PoolMaxEvents, size_t entrySize = sizeof(Type)) :
	mEventStorageCount(0),
	mMaxEvents(listSize),
	mEntrySize(entrySize)
	{
		mUnalignedStorage = (InternalPointerType)malloc((mMaxEvents * entrySize) + AlignmentValue);
		size_t storageAdress = (size_t) mUnalignedStorage;
		// Align the address
		if (AlignmentValue && (storageAdress % AlignmentValue))
		{
			size_t offset = storageAdress % AlignmentValue;
			mStoragePtr = (InternalPointerType)(storageAdress + AlignmentValue - offset);
		}
		else
		{
			mStoragePtr = mUnalignedStorage;
		}
		mCurrentStoragePositionPtr = mStoragePtr;
		mStorageEndPtr = (InternalPointerType)(((size_t)mStoragePtr) + (mMaxEvents * mEntrySize));
	}

	virtual ~EventPool()
	{
		free(mUnalignedStorage);
	}

	void append(MyType* pool)
	{
		size_t availableMemory = ((size_t)this->mStorageEndPtr - (size_t)this->mCurrentStoragePositionPtr);

		// precheck if even one event fits into this
		if (availableMemory < pool->memoryTo(0))
		{
			return;
		}

		size_t neededMemory = pool->memoryTo(pool->mEventStorageCount);

		if (availableMemory >= neededMemory)
		{
			// there is enough memory for the whole pool
			internalAppend(pool, pool->mEventStorageCount);

			pool->clear();
		}
		else
		{
			// put as much into this and leave the rest in pool
			size_t eventCount = 0, memory = 0;

			if(availableMemory < (neededMemory / 2))
			{
				// forward
				for (eventCount = 0; pool->memoryTo(eventCount) <= availableMemory; ++eventCount);
			}
			else
			{
				// backward
				for (eventCount = pool->mEventStorageCount - 1; pool->memoryTo(eventCount) > availableMemory; --eventCount);

				// we don't need the index, we need the count
				++eventCount;
			}

			internalAppend(pool, eventCount);

			// reorganize pool
			size_t startIndex = eventCount;
			size_t endIndex = pool->mEventStorageCount - 1;

			pool->reorganize(startIndex);
		}
	}

	//! \attention
	//! This only works for Events with constant size
	//! Respectively only copying pointers instead of a deep copy
	//! Deep copy is done, if Event is "flat"
	void copyTo(MyType* wRet) const
	{
#ifdef USE_SSE2
		nontemporal_copy
		(
			(char*)wRet->mStoragePtr,
			(char*)this->mStoragePtr,
			mMaxEvents * mEntrySize
		);
#else
		memcpy(wRet->mStoragePtr, this->mStoragePtr, mMaxEvents * mEntrySize);
#endif
		this->transferDataTo(wRet);
	}

	// Now its getting even stranger though
	// if you ever change this without asking me, your pc will crash .,.(o_o).,.
	MyType** copyTo(MyType** wRet, int count) const
	{
		BOOST_ASSERT(count < 8 && "8 is magic limit");
		char* outBuffer[8];
		for (int a = 0; a < count; ++a)
		{
			outBuffer[a] = (char*)wRet[a]->mStoragePtr;
		}
#ifdef USE_SSE2
		nontemporal_multi_copy
		(
			outBuffer,
			(char*)this->mStoragePtr,
			mMaxEvents * mEntrySize,
			count
		); 
#else
		for (int i = 0; i < count; ++i)
			memcpy(outBuffer[i], this->mStoragePtr, mMaxEvents * mEntrySize );
#endif
		for (int a = 0; a < count; ++a)
		{
			this->transferDataTo(wRet[a]);
		}
		return wRet;
	}

	void clear()
	{
		// Do manual Dtor call
		for (size_t i = 0; i < this->mEventStorageCount; ++i)
		{
			this->mStorageList[i] = 0;
		}
		// fastest "delete" ever
		mEventStorageCount = 0;
		mCurrentStoragePositionPtr = mStoragePtr;
	}

	TypePtr operator[](size_t pos) const
	{
		// its fast, not unsafe
		return pointerOfIndex(pos);
	}

	size_t size() const
	{
		return mEventStorageCount;
	}

	// \brief reserves a chunk of memory
	//
	// \param memory_size Size of the memory chunk to be reserved
	// \return Ptr to the memory
	//
	template < typename TargetType >
	TargetType* reserveNextStorage(size_t memory_size = 0)
	{
		memory_size = sizeof(TargetType);
		//
		// Should check alignment ? --> if ( memory_size < sizeof(size_t) ) memory_size = sizeof(size_t);
		// No !
		//
		TargetType* wRet = 0;
		if ((mEventStorageCount < mMaxEvents) && // need space in list
			( (((size_t)mCurrentStoragePositionPtr) + memory_size) <= (size_t)mStorageEndPtr) ) // need space in storage
		{
			// copy our ptr, because its also our return-value
			wRet = (TargetType*)mCurrentStoragePositionPtr;

			// create new entry in list and allocate memory
			mStorageList[mEventStorageCount] = ( ((size_t)mCurrentStoragePositionPtr) - ((size_t)mStoragePtr) );

			mEventStorageCount++;

			// seek for next free place in storage
			mCurrentStoragePositionPtr = 
				(InternalPointerType)( ((size_t)mCurrentStoragePositionPtr) + memory_size );

			wRet = new (wRet) TargetType();
		}
		return wRet;
	}

	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		// save the current byte count stored in this pool
		size_t memorySize = ((size_t) mCurrentStoragePositionPtr) - ((size_t) mStoragePtr);
		ar & memorySize;

		// save the bytes stored in this pool
		ar << boost::serialization::make_binary_object(mStoragePtr, memorySize);

		// save internal counter of events
		ar & mEventStorageCount;
		
		// save memory usage of this pool
		for (size_t i = 0; i < mEventStorageCount; ++i)
		{
			size_t listEntry = mStorageList[i];
			ar & listEntry;
		}
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		// restore the current byte count to be stored in this pool
		size_t memorySize;
		ar & memorySize;
		mCurrentStoragePositionPtr = (InternalPointerType)( memorySize + ((size_t)mStoragePtr) );

		// restore the bytes to be stored in this pool
		ar >> boost::serialization::make_binary_object(mStoragePtr, memorySize);

		// restore the internal counter of events to be stored in this pool
		ar & mEventStorageCount;

		// restore internal pointers to events
		for (size_t i = 0; i < mEventStorageCount; ++i)
		{
			size_t storage;
			ar & storage;
			mStorageList[i] = storage;
		}
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()

	friend std::ostream& operator << (std::ostream& os, MyType& es)
	{
		os << "EventPool(" << es.size() << " Events) ";
		for ( size_t i = 0; i < es.size(); ++i )
		{
			os << i << ":" << *(es[i]) << " ";
		}
		os << std::endl;
		return os;
	}

private:
	void transferDataTo(MyType* cloned) const
	{
		// transfer (calculate) mStorageList to wRet
		for (size_t i = 0; i < this->mEventStorageCount; ++i)
		{
			cloned->mStorageList[i] = this->mStorageList[i];
		}
		// set EventCount
		cloned->mEventStorageCount = this->mEventStorageCount;
		// calculate CurrentStoragePtr
		cloned->mCurrentStoragePositionPtr = 
			(InternalPointerType)( ((size_t)cloned->mStoragePtr) + 
			                       ((size_t)this->mCurrentStoragePositionPtr) -
			                       ((size_t)this->mStoragePtr) );
	}

	TypePtr pointerOfIndex(size_t index) const
	{
		if (index < mEventStorageCount)
		{
			return (TypePtr)(((size_t)mStoragePtr) + mStorageList[index]);
		}
		throw std::length_error("EventPool::pointerOfIndex: index out of range!");
	}

	size_t memoryTo(size_t eventIndex)
	{
		if ((eventIndex + 1) < mEventStorageCount)
		{
			return mStorageList[eventIndex + 1];
		}

		return (size_t)mCurrentStoragePositionPtr - (size_t)mStoragePtr;
	}

	// moves all events to the beginning (starting with startIndex)
	void reorganize(const size_t startIndex)
	{
		if (startIndex == 0)
			return;

		TypePtr startPtr = pointerOfIndex(startIndex);

		size_t memorySize = memoryTo(mEventStorageCount) - mStorageList[startIndex];

		// move the data
		memmove
		(
			mStoragePtr,
			startPtr,
			memorySize
		);

		// save and reset count
		size_t oldEventCount = mEventStorageCount;
		mEventStorageCount = 0;

		// copy internal pointers to the data
		for (size_t i = startIndex; i < oldEventCount; ++i, ++this->mEventStorageCount)
		{
			mStorageList[mEventStorageCount] = mStorageList[i] - mStorageList[startIndex];
		}

		this->mCurrentStoragePositionPtr = (InternalPointerType)((size_t)mStoragePtr + memorySize);
	}

	// appends events from another pool into this (endIndex not included)
	void internalAppend(MyType* pool, size_t endIndex)
	{
		// calculate important values before overwriting this
		if (endIndex <= 0)
			throw std::logic_error("EventPool::internalAppend: endIndex less than or equal to 0!");

		size_t memorySize = pool->mStorageList[endIndex - 1];
		size_t currentSize = this->memoryTo(this->mEventStorageCount);

		// copy internal pointers to the data
		for (size_t i = 0; i < endIndex; ++i, ++this->mEventStorageCount)
		{
			size_t newListEntry = pool->mStorageList[i] + currentSize;

			this->mStorageList[this->mEventStorageCount] = newListEntry;
		}

		// copy the data itself
		memcpy
		(
			this->mCurrentStoragePositionPtr, 
			pool->mStoragePtr,
			memorySize
		);

		size_t current = (size_t)this->mCurrentStoragePositionPtr;
		this->mCurrentStoragePositionPtr = (InternalPointerType)(current + memorySize);
	}

#ifdef USE_SSE2
	void nontemporal_copy(char* outbuff, char* inbuff, int size)
	{
		const int step = 64; // cache line
		while(size > step)
		{
			_mm_prefetch(inbuff + 320, _MM_HINT_NTA); // non-temporal prefetch
			__m128i A = _mm_loadu_si128((__m128i*) (inbuff +  0));
			__m128i B = _mm_loadu_si128((__m128i*) (inbuff + 16));
			__m128i C = _mm_loadu_si128((__m128i*) (inbuff + 32));
			__m128i D = _mm_loadu_si128((__m128i*) (inbuff + 48));

			// destination address must be 16-byte aligned!
			_mm_stream_si128((__m128i*) (outbuff +  0), A);
			_mm_stream_si128((__m128i*) (outbuff + 16), B);
			_mm_stream_si128((__m128i*) (outbuff + 32), C);
			_mm_stream_si128((__m128i*) (outbuff + 48), D);

			inbuff  += step;
			outbuff += step;
			size -= step;
		}
		_mm_mfence();
	}

	void nontemporal_multi_copy(char** outbuff, char* inbuff, int size, const int count)
	{
		const int step = 64; // cache line

		char* outbufferCopy [4];
		int j = 0;
		while ( j <= count )
		{
			outbufferCopy[j] = outbuff[j];
			++j;
		}

		while(size > step)
		{
			_mm_prefetch(inbuff + 320, _MM_HINT_NTA); // non-temporal prefetch
			__m128i A = _mm_loadu_si128((__m128i*) (inbuff +  0));
			__m128i B = _mm_loadu_si128((__m128i*) (inbuff + 16));
			__m128i C = _mm_loadu_si128((__m128i*) (inbuff + 32));
			__m128i D = _mm_loadu_si128((__m128i*) (inbuff + 48));

			int i = 0;
			while ( i < count )
			{
				// destination address must be 16-byte aligned!
				_mm_stream_si128((__m128i*) (outbufferCopy[i] +  0), A);
				_mm_stream_si128((__m128i*) (outbufferCopy[i] + 16), B);
				_mm_stream_si128((__m128i*) (outbufferCopy[i] + 32), C);
				_mm_stream_si128((__m128i*) (outbufferCopy[i] + 48), D);
				outbufferCopy[i] += step;
				++i;
			}
			inbuff  += step;

			size -= step;
		}
		_mm_mfence();
	}
#endif //USE_SSE2

	size_t mEventStorageCount;
	InternalPointerType mUnalignedStorage;
	InternalPointerType mStoragePtr;
	InternalPointerType mCurrentStoragePositionPtr;
	InternalPointerType mStorageEndPtr;

	size_t mMaxEvents;
	size_t mEntrySize;

	int * mUseLess;
	// The storage itself
	size_t mStorageList[PoolMaxEvents];

};

//typedef EventPool<BaseEvent> BaseEventPool;
typedef EventPool<IEvent> BaseEventPool;

#endif // __EVENT_POOL_H_

