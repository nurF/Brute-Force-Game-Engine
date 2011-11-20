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

#include "Archiver.h"
#include <memory.h>

VirtualFile::VirtualFile( ArchivContainer * aCont ) : io::stream<VirtualFileDevice>( aCont, this ),
	cont( aCont ), hndIdx( 0xFFFFFFFF ), dirIdx( 0xFFFFFFFF ), firstChunk( NULL ), lastChunk( NULL ), localPos( 0 ),
	fileSize( 0 ), curNextChunk( NULL ), bModified( false )
{
}

VirtualFile::~VirtualFile()
{
	deleteCache();
}

void VirtualFile::fileClose()
{
	flush();

	cont->openFileHandles[hndIdx] = NULL;
	hndIdx = 0xFFFFFFFF;

	if( !isModified() )
	{
		cont->catalogue[dirIdx]->ref = NULL;
		delete this;
	}
	else
	{
		mergeBack();

		// keep on hold, for writing at end

		// TODO: ...
	}
}

void VirtualFile::deleteCache()
{
	// cleanup all chunks
	while( firstChunk != NULL )
		deleteChunk( firstChunk );
	curNextChunk = NULL;
}

VirtualFile::ShadowChunk * VirtualFile::injectChunk()
{
	ShadowChunk * newChunk = createChunkAt( curNextChunk );
	newChunk->localStartPos = localPos;
	newChunk->localEndPos = localPos;
	curNextChunk = newChunk;

	return newChunk;
}

void VirtualFile::deleteChunk( ShadowChunk * p )
{
	if( p->prev != NULL )
		p->prev->next = p->next;
	else
		firstChunk = p->next;
	if( p->next != NULL )
		p->next->prev = p->prev;
	else
		lastChunk = p->prev;

	delete p;
}

VirtualFile::ShadowChunk * VirtualFile::createChunkAt( ShadowChunk * before )
{
	ShadowChunk * newChunk = new ShadowChunk;

	if( before != NULL )
	{
		newChunk->next = before;
		newChunk->prev = before->prev;
		before->prev = newChunk;
		if( newChunk->prev != NULL )
			newChunk->prev->next = newChunk;
		else
			firstChunk = newChunk;
	}
	else
	{
		newChunk->prev = lastChunk;
		newChunk->next = NULL;
		if( lastChunk != NULL )
			lastChunk->next = newChunk;
		else
			firstChunk = newChunk;
		lastChunk = newChunk;
	}

	return newChunk;
}

void VirtualFile::mergeBack()
{
	if( firstChunk != NULL )
	{
		if( firstChunk->localStartPos == 0 &&
			firstChunk->localEndPos == fileSize )
		{
			assert( firstChunk == lastChunk );
			return;	// Already done.
		}
	}
	else if( fileSize <= 0 )
		return;	// Already done.

	// merges ShadowChunk fragments with backend content into a single ShadowChunk
	char * readFragment = new char[MAX_MERGE_FRAGSIZE];

	// seek to start
	seekFile( 0, std::ios::beg );

	// tricky data ttransfer to big chunk, by simply read and write data immediately
	size_t transferAmount;
	size_t errorAmount = 0;
	while( localPos < fileSize )
	{
		transferAmount = fileSize - localPos;
		if( transferAmount > MAX_MERGE_FRAGSIZE )
			transferAmount = MAX_MERGE_FRAGSIZE;

		transferAmount = readFile( readFragment, transferAmount );
		if( transferAmount <= 0 )
		{
			// error occured
			errorAmount ++;
			transferAmount = 1;
			writeCache( "\0", 1 );
		}
		else
		{
			// write back
			seekFile( -(io::stream_offset)transferAmount, std::ios::cur );
			writeCache( readFragment, transferAmount );
		}
	}

	if( errorAmount > 0 )
	{
		printf( "WARNING: %d bytes couldn't be read from backend device\n", (int)errorAmount );
	}

	delete[] readFragment;
}

std::streamsize VirtualFile::readBackend(VirtualFileDevice::char_type* s, std::streamsize n)
{
	if( !cont->backend )
		return 0;
	cont->backend->seekg( backendPos + localPos, std::ios::beg );
	size_t res = cont->backend->read( s, n );

	localPos += res;
	return res;
}

std::streamsize VirtualFile::writeBackend(const VirtualFileDevice::char_type* s, std::streamsize n)
{
	if( !cont->backend )
		return 0;
	cont->backend->seekg( backendPos + localPos, std::ios::beg );

	size_t res = cont->backend->write( s, n );

	localPos += res;
	return res;
}

std::streamsize VirtualFile::readCache(char_type* s, std::streamsize n)
{
	if( curNextChunk == NULL )
		return NULL;

	size_t readStep;
	size_t begPos;

	readStep = curNextChunk->localEndPos - localPos;
	if( readStep > n )
		readStep = n;

	begPos = localPos - curNextChunk->localStartPos;
	memcpy( s, &curNextChunk->memory[begPos], readStep );

	localPos += readStep;
	return readStep;
}

std::streamsize VirtualFile::readFile(char_type* s, std::streamsize n)
{
	size_t readSize = fileSize - localPos;
	if( n > readSize )
		n = readSize;

	// read from chunks or from backend stream
	size_t readStep, res, begPos;

	readSize = 0;
	while( n > 0 )
	{
		readStep = 0;

		if( curNextChunk != NULL )
		{
			if( curNextChunk->localStartPos > localPos )
			{
				// read from backend
				readStep = curNextChunk->localStartPos - localPos;
				if( readStep > n )
					readStep = n;

				res = readBackend( s, readStep );
				if( res < readStep )
				{
					// error! Quit reading.
					readStep = res;
					n = res;
				}
			}
			else
			{
				if( curNextChunk->localEndPos > localPos )
				{
/*					// read from chunk
					readStep = cur->localEndPos - localPos;
					if( readStep > n )
						readStep = n;

					begPos = localPos - cur->localStartPos;
					memcpy( s, &cur->memory[begPos], readStep );*/

					readStep = readCache( s, n );
				}
				else
				{
					// we may do it, because n > 0
					curNextChunk = curNextChunk->next;
				}
			}
		}

		if( curNextChunk == NULL )
		{
			// read from backend
			readStep = n;

			res = readBackend( s, readStep );
			if( res < readStep )
			{
				// error! Quit reading.
				readStep = res;
				n = res;
			}
		}

		// localPos += readStep; // MOVED
		s += readStep;
		readSize += readStep;
		n -= readStep;
	}

	return readSize;
}

std::streamsize VirtualFile::writeCache(const char_type* s, std::streamsize n)
{
	assert( localPos >= 0 && localPos <= fileSize );

	// allocate a new chunk to write to
	if( curNextChunk == NULL )
		injectChunk();

	if( localPos == fileSize )
	{
		// append
		if( curNextChunk->memory.capacity() < curNextChunk->memory.size() + n )
			curNextChunk->memory.reserve( 2 * curNextChunk->memory.capacity() + n );
		curNextChunk->memory.insert( curNextChunk->memory.end(), s, s + n );
		curNextChunk->localEndPos += n;
	}
	else
	{
		// overwrite
		if( curNextChunk->localStartPos > localPos )
			injectChunk();	// create a new one to write to

		size_t begPos = localPos - curNextChunk->localStartPos;
		size_t curSize = curNextChunk->memory.size();
		size_t addSize;
		if( begPos + n > curSize )
			addSize = begPos + n - curSize;
		else
			addSize = 0;

		// overwrite in chunk
		size_t replaceSize = n - addSize;
		if( replaceSize > 0 )	// prevents assertions
			memcpy( &curNextChunk->memory[begPos], s, replaceSize );

		// append last characters
		if( curNextChunk->memory.capacity() < curNextChunk->memory.size() + n - replaceSize )
			curNextChunk->memory.reserve( 2 * curNextChunk->memory.capacity() + n - replaceSize );
		curNextChunk->memory.insert( curNextChunk->memory.end(), s + replaceSize, s + n );
		curNextChunk->localEndPos += addSize;

		// swallow following chunks
		VirtualFile::ShadowChunk * cur, * next;
		for( cur = curNextChunk->next; cur != NULL; cur = next )
		{
			next = cur->next;
			if( cur->localEndPos <= curNextChunk->localEndPos )
			{
				// remove chunk
				deleteChunk( cur );
			}
			else if( cur->localStartPos <= curNextChunk->localEndPos )
			{
				// merge chunk
				begPos = curNextChunk->localEndPos - cur->localStartPos;
				addSize = cur->memory.size() - begPos;

				curNextChunk->memory.insert( curNextChunk->memory.end(),
											 cur->memory.begin() + begPos, cur->memory.end() );
				curNextChunk->localEndPos += addSize;

				deleteChunk( cur );
				break;
			}
		}
	}

	localPos += n;
	if( fileSize < localPos )
		fileSize = localPos;

	if( n > 0 )
	{
		bModified = true;
		cont->bModified = true;
	}

	return n;
}

io::stream_offset VirtualFile::seekFile(io::stream_offset off, std::ios_base::seekdir way)
{
	// Determine new value of pos_
	io::stream_offset nextPos;
	if (way == std::ios_base::beg)
		nextPos = off;
	else if (way == std::ios_base::cur)
		nextPos = localPos + off;
	else if (way == std::ios_base::end)
		nextPos = fileSize + off;
	else
		throw std::ios_base::failure("bad seek direction");

	// Check for errors
	if( nextPos < 0 || nextPos > fileSize )
		throw std::ios_base::failure("bad seek offset");

	// find chunk
	VirtualFile::ShadowChunk * cur, * prev;
	if( nextPos > localPos )
	{
		for( cur = curNextChunk; cur != NULL; cur = cur->next )
		{
			if( cur->localEndPos >= nextPos )
				break;
		}
		curNextChunk = cur;
	}
	else if( nextPos < localPos )
	{
		cur = curNextChunk;
		if( cur == NULL )
			cur = lastChunk;
		if( cur != NULL )
		{
			while( 1 )
			{
				prev = cur->prev;
				if( prev == NULL )
					break;
				if( prev->localEndPos >= nextPos )
					cur = prev;
				if( prev->localStartPos <= nextPos )
					break;
			}
			curNextChunk = cur;
		}
	}

	localPos = nextPos;
	return nextPos;
}

//////////////////////////////////////////////////////////////////////////////////
// Members of ArchivContainer
//////////////////////////////////////////////////////////////////////////////////

ArchivContainer::ArchivContainer() : backend( NULL ), bModified( false )
{
	openFileHandles.reserve( STD_SIMULTAN_OPEN );
}

ArchivContainer::~ArchivContainer()
{
	// write back modified
	flushBuffer();
	clearContent();

	if( backend != NULL )
		delete backend;
}

void ArchivContainer::clearContent()
{
	// Close all open files
	VirtualFile_ptrVec::iterator iter;
	for( iter = openFileHandles.begin(); iter != openFileHandles.end(); iter ++ )
	{
		if( *iter )
			(*iter)->fileClose();
	}

	// Clear catalogue
	Directory_ptrVec::iterator iter2;
	for( iter2 = catalogue.begin(); iter2 != catalogue.end(); iter2 ++ )
	{
		if( (*iter2)->ref )
			delete (*iter2)->ref;
		delete (*iter2);
	}

	catalogue.clear();
	openFileHandles.clear();
	fileMap.clear();
}

void ArchivContainer::cleanupSlots()
{
	// trim NULLs at end
	VirtualFile_ptrVec::reverse_iterator iter;

	for( iter = openFileHandles.rbegin(); iter != openFileHandles.rend(); iter ++ )
	{
		if( *iter != NULL )
			break;
	}

	size_t numClear = iter - openFileHandles.rbegin();
	openFileHandles.erase( openFileHandles.end() - numClear, openFileHandles.end() );
}

VirtualFile * ArchivContainer::fileOpen( std::string fileName )
{
	if( !backend )
		return NULL; // backend should exist!

	// search in virtual catalogue
	VirtualFile * newHandle;
	Directory * dir;

	strIdx_map::iterator iter = fileMap.find( fileName );
	if( iter != fileMap.end() )
	{
		dir = catalogue[iter->second];
		if( dir->ref != NULL && dir->ref->hndIdx != 0xFFFFFFFF )
			return NULL;	// already open. Give Error
	}
	else
		dir = NULL;

	// get a file handler
	cleanupSlots();
	size_t fileHandleID = openFileHandles.size();
	if( dir == NULL || dir->ref == NULL )
	{
		newHandle = new VirtualFile( this );

		// init handle
		if( dir == NULL )
		{
			// new file
			size_t dirIdx = catalogue.size();
			catalogue.resize( dirIdx + 1 );
			dir = new Directory;
			catalogue[dirIdx] = dir;
			fileMap.insert( std::pair<std::string, size_t>( fileName, dirIdx ) );

			dir->index = dirIdx;
			dir->lumpSize = 0;
			dir->fileSize = 0;
			dir->dataPos = 0;
			dir->fileName = fileName;
			dir->bIsDeleted = false;

			newHandle->backendSize = 0;
			newHandle->fileSize = 0;
			newHandle->backendPos = 0;
			newHandle->dirIdx = dirIdx;

			// Set modified flag
			newHandle->injectChunk();
			newHandle->bModified = true;
			bModified = true;
		}
		else
		{
			// open file
			newHandle->backendSize = dir->fileSize;
			newHandle->fileSize = dir->fileSize;
			newHandle->backendPos = dir->dataPos;
			newHandle->dirIdx = iter->second;
		}

		dir->ref = newHandle;
	}
	else
		newHandle = dir->ref;

	openFileHandles.push_back( newHandle );
	newHandle->hndIdx = fileHandleID;

	return newHandle;
}

bool ArchivContainer::deleteFile( std::string fileName )
{
	Directory * dir;

	strIdx_map::iterator iter = fileMap.find( fileName );
	if( iter == fileMap.end() )
		return false;

	dir = catalogue[iter->second];
	if( dir->ref != NULL && dir->ref->hndIdx != 0xFFFFFFFF )
		return false;	// already open. Give Error

	if( dir->ref != NULL )
	{
		delete dir->ref;
		dir->ref = NULL;
	}

	dir->bIsDeleted = true;
	return true;
}

size_t ArchivContainer::getFileCount() const
{
	// TODO: ignores NULL entries and deleted files!
	return catalogue.size();
}

const char * ArchivContainer::getFileName( size_t atIndex ) const
{
	// TODO: ignores NULL entries and deleted files! May lead to crash!
	return catalogue[atIndex]->fileName.data();
}

static int _sortCat( const void * A, const void * B )
{
	if( ((ArchivContainer::Directory *)B)->dataPos < ((ArchivContainer::Directory *)A)->dataPos )
		return 1;
	else if( ((ArchivContainer::Directory *)B)->dataPos > ((ArchivContainer::Directory *)A)->dataPos )
		return -1;
	else
		return 0;
}

void ArchivContainer::flushBuffer()
{
	// Flush and merge frontend files and count files
	Directory_ptrVec::iterator iter;
	Directory * dir;
	size_t numFiles = 0;

	for( iter = catalogue.begin(); iter != catalogue.end(); iter ++ )
	{
		dir = *iter;
		if( dir == NULL )
			continue;
		if( dir->bIsDeleted )
			continue;
		if( dir->ref == NULL )
		{
			numFiles ++;
			continue;
		}

		// flush & merge modified files
		dir->ref->flush();
		if( dir->ref->bModified )
		{
			dir->ref->mergeBack();

			// after merging, only single chunk should exist containing entire file
			assert( dir->ref->firstChunk != NULL );
			assert( dir->ref->firstChunk == dir->ref->lastChunk );
			assert( dir->ref->firstChunk->localStartPos == 0 );
			assert( dir->ref->firstChunk->localEndPos == dir->ref->fileSize );
		}

		numFiles ++;
	}

	// If not modified, reject
	if( !bModified )
		return;
	bModified = false;

	// write file count to frontend
	size_t dirIdx = 0;
	size_tbig backendFileSize = 0;
	long lnVal;
	size_t headerSize;

	backend->seekg( 0, std::ios::beg );
	lnVal = numFiles;
	backend->write( (const char *)&lnVal, sizeof( long ) );
	backendFileSize = backend->tellg();

	// write unmodified frontend files back
	io::stream_offset headerBegPos, dataBegPos, endPos;
	long default0 = 0;

	VirtualFile * file, * opened;
	size_t transferAmount;
	size_t errorAmount;
	char * readFragment = new char[MAX_MERGE_FRAGSIZE];

	for( iter = catalogue.begin(); iter != catalogue.end(); iter ++ )
	{
		dir = *iter;
		if( dir == NULL )
			continue;
		if( dir->bIsDeleted )
			continue;
		file = dir->ref;
		if( file != NULL && file->isModified() )
			continue;

		if( dir->index == dirIdx )
		{
			dirIdx ++;

			// Skip
			backendFileSize += dir->lumpSize;
			backend->seekg( dir->lumpSize, std::ios::cur );
			continue;
		}

		// Assure it's open file
		if( file == NULL )
			file = opened = fileOpen( dir->fileName );
		else
			opened = NULL;

		// No shadow chunks allowed for unmodified files
		assert( file->firstChunk == NULL );

		// append header
		///////////////////

		// HACK: ignoring errors!
		headerBegPos = backend->tellg();
		backend->write( (const char *)&default0, sizeof( long ) );	// keep 4 bytes free for headerSize
		backend->write( (const char *)&default0, sizeof( long ) );	// keep 4 bytes free for lumpSize
		lnVal = dir->fileName.size();
		backend->write( (const char *)&lnVal, sizeof( long ) );
		backend->write( (const char *)dir->fileName.data(), lnVal );

		// append data
		///////////////////
		dataBegPos = backend->tellg();

		// seek to start
		errorAmount = 0;
		file->seekFile( 0, std::ios::beg );

		// data transfer
		endPos = backend->tellg();
		while( file->localPos < file->fileSize )
		{
			transferAmount = file->fileSize - file->localPos;
			if( transferAmount > MAX_MERGE_FRAGSIZE )
				transferAmount = MAX_MERGE_FRAGSIZE;

			transferAmount = file->readBackend( readFragment, transferAmount );
			if( transferAmount <= 0 )
			{
				// error occured
				errorAmount ++;

				backend->seekg( endPos, std::ios::beg );
				backend->write( "\0", 1 );
				file->seekFile( 1, std::ios::cur );
			}
			else
			{
				// write back
				backend->seekg( endPos, std::ios::beg );
				backend->write( readFragment, transferAmount );
			}

			endPos = backend->tellg();
		}

		if( errorAmount > 0 )
		{
			printf( "WARNING: %d bytes couldn't be read from backend device\n", (int)errorAmount );
		}

		// alter directory element
		dir->index = dirIdx;

		headerSize = dataBegPos - headerBegPos;
		dir->lumpSize = endPos - headerBegPos;
		assert( dir->fileSize == endPos - dataBegPos );
		dir->dataPos = dataBegPos;

		// alter file handler
		file->dirIdx = dirIdx;
		assert( file->fileSize == dir->fileSize );
		assert( file->backendSize == dir->fileSize );

		// write headerSize and lumpSize to header
		backend->seekg( headerBegPos, std::ios::beg );
		lnVal = headerSize;
		backend->write( (char *)&lnVal, sizeof( long ) );
		lnVal = dir->lumpSize;
		backend->write( (char *)&lnVal, sizeof( long ) );

		// seek to end
		backend->seekg( endPos, std::ios::beg );

		// close opened file
		if( opened )
			opened->fileClose();

		backendFileSize = endPos;
		dirIdx ++;
	}
	
	// append new files
	for( iter = catalogue.begin(); iter != catalogue.end(); iter ++ )
	{
		dir = *iter;
		if( dir == NULL )
			continue;
		if( dir->bIsDeleted )
			continue;
		file = dir->ref;
		if( file == NULL )
			continue;
		if( !file->isModified() )
			continue;

		// after merging, only single chunk should exist containing entire file
		assert( file->firstChunk != NULL );
		assert( file->firstChunk == file->lastChunk );
		assert( file->firstChunk->localStartPos == 0 );
		assert( file->firstChunk->localEndPos == file->fileSize );

		// append header
		///////////////////

		// HACK: ignoring errors!
		headerBegPos = backend->tellg();
		backend->write( (const char *)&default0, sizeof( long ) );	// keep 4 bytes free for headerSize
		backend->write( (const char *)&default0, sizeof( long ) );	// keep 4 bytes free for lumpSize
		lnVal = dir->fileName.size();
		backend->write( (const char *)&lnVal, sizeof( long ) );
		backend->write( (const char *)dir->fileName.data(), lnVal );

		// append data
		///////////////////
		dataBegPos = backend->tellg();

		// seek to start
		errorAmount = 0;
		file->seekFile( 0, std::ios::beg );

		// data transfer
		while( file->localPos < file->fileSize )
		{
			transferAmount = file->fileSize - file->localPos;
			if( transferAmount > MAX_MERGE_FRAGSIZE )
				transferAmount = MAX_MERGE_FRAGSIZE;

			transferAmount = file->readCache( readFragment, transferAmount );
			if( transferAmount <= 0 )
			{
				// error occured
				errorAmount ++;

				backend->write( "\0", 1 );
				file->seekFile( 1, std::ios::cur );
			}
			else
			{
				// write back
				backend->write( readFragment, transferAmount );
			}
		}
		endPos = backend->tellg();

		if( errorAmount > 0 )
		{
			printf( "WARNING: %d bytes couldn't be read from frontend cache\n", (int)errorAmount );
		}

/*		// write headerSize and lumpSize
		backend->seekg( headerBegPos, std::ios::beg );
		size = dataBegPos - headerBegPos;
		backend->write( (char *)&size, sizeof( long ) );
		size = endPos - dataBegPos;
		backend->write( (char *)&size, sizeof( long ) );

		// seek to end
		backend->seekg( endPos, std::ios::beg );
		backendFileSize = endPos;*/

		// alter directory element
		dir->index = dirIdx;
		headerSize = dataBegPos - headerBegPos;
		dir->lumpSize = endPos - headerBegPos;
		dir->fileSize = endPos - dataBegPos;
		dir->dataPos = dataBegPos;

		// alter file handler
		file->dirIdx = dirIdx;
		file->fileSize = dir->fileSize;
		file->backendSize = dir->fileSize;

		// write headerSize and lumpSize to header
		backend->seekg( headerBegPos, std::ios::beg );
		lnVal = headerSize;
		backend->write( (char *)&lnVal, sizeof( long ) );
		lnVal = dir->lumpSize;
		backend->write( (char *)&lnVal, sizeof( long ) );

		// seek to end
		backend->seekg( endPos, std::ios::beg );
		backendFileSize = endPos;

		dirIdx ++;
	}

	delete[] readFragment;

	// truncate
	backend->truncate( backendFileSize );

	// tidy up catalogue
	for( iter = catalogue.begin(); iter != catalogue.end(); iter ++ )
	{
		dir = *iter;

		// clear dead files
		if( dir == NULL || dir->bIsDeleted )
		{
			assert( dir->ref == NULL );

			// Remove zombie entries
			catalogue.erase( iter );
			iter = catalogue.begin();
			continue;
		}

		// Clear cache
		file = dir->ref;
		if( file != NULL )
		{
			if( file->bModified )
			{
				file->bModified = false;
				file->deleteCache();
			}
		}
	}

	// resort elements
	qsort( &catalogue[0], catalogue.size(), sizeof( Directory * ), _sortCat );
}

void ArchivContainer::scanCatalogue()
{
	// TODO: Possibly untrusted data!!! Should do checks to prevent DoS

	if( !backend )
		return;

	// read catalogue from backEnd
	backend->seekg( 0, std::ios::beg );
	long numCatalogueItems = 0;

	if( backend->read( (char *)&numCatalogueItems, sizeof( long ) ) < sizeof( long ) )
		return;	// empty
	catalogue.reserve( numCatalogueItems );

	long nameSize, lumpSize, headerSize;
	io::stream_offset pos, begPos, d;
	char * filename = new char[MAX_FILENAME + 1];
	size_t index = 0;

	while( numCatalogueItems > 0 )
	{
		begPos = backend->tellg();
		headerSize = 0;

		// read headerSize
		if( backend->read( (char *)&headerSize, sizeof( long ) ) < sizeof( long ) )
			break;

		// read lumpsize (32bit cap)
		if( backend->read( (char *)&lumpSize, sizeof( long ) ) < sizeof( long ) )
			break;

		// read filename
		if( backend->read( (char *)&nameSize, sizeof( long ) ) < sizeof( long ) )
			break;
		if( nameSize > MAX_FILENAME )
		{
			printf( "ERROR: Catalogue contains a filename, which is too long.\n" );
			break;
		}
		if( backend->read( filename, nameSize ) < nameSize )
			break;

		// TODO: Verify size values!!! DoS attack vulnerablility!

		// position
		pos = backend->tellg();

		// skip lump
		d = pos - begPos;
		if( d > headerSize )
		{
			printf( "ERROR: Corrupt header in catalogue\n" );
			break;
		}
		backend->seekg( lumpSize - headerSize, std::ios::cur );

		// TODO: create directory entry
		size_t idx = catalogue.size();
		catalogue.resize( idx + 1 );
		Directory * dir = new Directory;
		catalogue[idx] = dir;

		dir->bIsDeleted = false;
		dir->dataPos = pos;
		dir->fileName.assign( filename, nameSize );
		dir->fileSize = lumpSize - headerSize;
		dir->index = idx;
		dir->lumpSize = lumpSize;
		dir->ref = NULL;
		fileMap.insert( std::pair<std::string, size_t> ( dir->fileName, idx ) );

		numCatalogueItems --;
	}

	delete[] filename;

	if( numCatalogueItems > 0 )
	{
		printf( "ERROR: Couldn't read whole catalogue! Some files will be missing\n" );
	}

/*	// Verify that all items are ordered by Directory::dataPos
	if( catalogue.size() >= 2 )
	{
		Directory_ptrVec::iterator iter;
		for( iter = catalogue.begin() + 1; iter != catalogue.end(); iter ++ )
		{
			if( (*(iter - 1))->dataPos >= (*(iter))->dataPos )
			{
				printf( "ERROR: Misordered files\n" );
			}
		}
	}*/
}
