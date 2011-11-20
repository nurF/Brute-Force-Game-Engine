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

#ifndef _BFGARCHIVER_H
#define _BFGARCHIVER_H

#ifdef _MSC_VER
#pragma warning( disable: 4996 )	// This will stop all nasty unsafe POSIX methods warnings
#endif

#include <map>
#include <vector>
#include <string>

#include <boost/iostreams/stream.hpp>

namespace io = boost::iostreams;

#define STD_SIMULTAN_OPEN	50
#define MAX_MERGE_FRAGSIZE	2048
#define MAX_FILENAME		1024
// #define CACHE_RESIZE_DELTA	4096

typedef std::vector<char> char_vec;
typedef unsigned long long uint64;	// GCC doesn't know __int64!

typedef uint64 size_tbig;

// Device - A shell to access virtual file through boost streams
/////////////////////////////////////////

class ArchivContainer;
class VirtualFile;
class VirtualFileDevice
{
public:
    typedef char						char_type;
	typedef io::seekable_device_tag     category;

	VirtualFileDevice( ArchivContainer * aCont, VirtualFile * aFile ) : cont( aCont ), file( aFile )
	{
	}
	virtual ~VirtualFileDevice()
	{
	}

    inline std::streamsize read(char_type* s, std::streamsize n);
    inline std::streamsize write(const char_type* s, std::streamsize n);
	inline io::stream_offset seek(io::stream_offset off, std::ios_base::seekdir way);

private:
	ArchivContainer * cont;
	VirtualFile * file;
};

// Virtual file - stream implementation and openable by archive frontend interface
/////////////////////////////////////////

class VirtualFile : public io::stream<VirtualFileDevice>
{
	friend class ArchivContainer;
	friend class VirtualFileDevice;

protected:
	VirtualFile( ArchivContainer * aCont );
	virtual ~VirtualFile();

public:
	void fileClose();
	inline bool isModified() { return bModified; }

private:
	// Shadow chunks for caching write access differences
	struct ShadowChunk
	{
		// TODO: Allow temporary file chunks for very huge sizes

		ShadowChunk * next;
		ShadowChunk * prev;

		char_vec memory;
		size_t localStartPos;
		size_t localEndPos;
	};
	ShadowChunk * injectChunk();
	void deleteChunk( ShadowChunk * p );
	ShadowChunk * createChunkAt( ShadowChunk * before = NULL );
	void mergeBack();	// merges ShadowChunk fragments with backend content into a single ShadowChunk
	void deleteCache();

	// Backend access
	std::streamsize readBackend(VirtualFileDevice::char_type* s, std::streamsize n);
    std::streamsize writeBackend(const VirtualFileDevice::char_type* s, std::streamsize n);

	// File access. (Both cache and backend)
    std::streamsize readFile(char_type* s, std::streamsize n);
	io::stream_offset seekFile(io::stream_offset off, std::ios_base::seekdir way);

	// Cache access
    std::streamsize readCache(char_type* s, std::streamsize n);
    std::streamsize writeCache(const char_type* s, std::streamsize n);

private:
	ArchivContainer * cont;
	size_t hndIdx;
	size_t dirIdx;
	bool bModified;

	// cache
	ShadowChunk * firstChunk;
	ShadowChunk * lastChunk;

	// position & size
	size_t localPos;
	size_t fileSize;
	ShadowChunk * curNextChunk;

	// backend stream entry
	io::stream_offset backendPos;
	size_t backendSize;					// if 0 then backend isn't existing

	// ....
};

typedef std::vector<VirtualFile *> VirtualFile_ptrVec;

///// //// ////

inline std::streamsize VirtualFileDevice::read(VirtualFileDevice::char_type* s, std::streamsize n)
{
	std::streamsize sz = file->readFile( s, n );

	// EOF
	if( sz == 0 && file->localPos == file->fileSize )
		return -1;
	return sz;
}
inline std::streamsize VirtualFileDevice::write(const VirtualFileDevice::char_type* s, std::streamsize n)
{
	return file->writeCache( s, n );
}
inline io::stream_offset VirtualFileDevice::seek(io::stream_offset off, std::ios_base::seekdir way)
{
	return file->seekFile( off, way );
}

// Archiv itself - splits a backend stream into virtual files at frontend
/////////////////////////////////////////

class ArchivContainer
{
	friend class VirtualFile;
public:
	ArchivContainer();
	virtual ~ArchivContainer();

	// backend methods
	template<class T>
	bool assignBackend( T& newBackend )
	{
		if( bModified || openFileHandles.size() > 0 )
			return false;	// invalid operation
		if( backend != NULL )
			delete backend;
		backend = new BackendInterface_Impl<T>( newBackend );

		clearContent();
		scanCatalogue();
		return true;
	}
	void flushBuffer();

	// frontend methods
	VirtualFile * fileOpen( std::string fileName );
	bool deleteFile( std::string fileName );
	size_t getFileCount() const;
	const char * getFileName( size_t atIndex ) const;
	
public:
	struct Directory
	{
		size_t index;

		std::string fileName;	// if 0, then file is new
		size_t lumpSize;
		size_t fileSize;		// fileSize <= lumpSize - headerSize with padding
		io::stream_offset dataPos;
		VirtualFile * ref;

		bool bIsDeleted;
	};

private:
	void scanCatalogue();
	void cleanupSlots();
	void clearContent();

	// Backend access
	struct BackendInterface
	{
		virtual std::streamsize read(VirtualFileDevice::char_type* s, std::streamsize n) = 0;
		virtual std::streamsize write(const VirtualFileDevice::char_type* s, std::streamsize n) = 0;
		virtual io::stream_offset seekg(io::stream_offset off, std::ios::seekdir way) = 0;
		virtual io::stream_offset tellg() = 0;
		virtual size_tbig truncate( size_tbig fileSize ) = 0;
	};

	template<class T>
	struct BackendInterface_Impl : public BackendInterface
	{
		BackendInterface_Impl( T& aStrm ) : strm_( &aStrm ) {}
		std::streamsize read(VirtualFileDevice::char_type* s, std::streamsize n)
		{
/*			io::stream_offset oldPos = strm_->tellg();
			strm_->read( s, n );
			return strm_->tellg() - oldPos;*/
			return strm_->readsome( s, n );
		}
		std::streamsize write(const VirtualFileDevice::char_type* s, std::streamsize n)
		{
			io::stream_offset oldPos = strm_->tellg();
			strm_->write( s, n );
			return strm_->tellg() - oldPos;
		}
		io::stream_offset seekg(io::stream_offset off, std::ios::seekdir way)
		{
			strm_->seekg( off, way );
			return strm_->tellg();
		}
		io::stream_offset tellg()
		{
			return strm_->tellg();
		}
		size_tbig truncate( size_tbig fileSize )
		{
			// TODO: ...
			return 0;
		}

		T * strm_;
	};

private:
	typedef std::vector<Directory *> Directory_ptrVec;
	typedef std::map<std::string, size_t> strIdx_map;

	strIdx_map fileMap;
	Directory_ptrVec catalogue;
	VirtualFile_ptrVec openFileHandles;
	BackendInterface * backend;
	bool bModified;
};

#endif
