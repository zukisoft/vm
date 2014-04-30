//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "FileDescriptorTable.h"

#pragma warning(push, 4)			

// FD_NUM_RESERVED
//
// Defines the number of file descriptor indexes that are reserved for
// specific reasons (STDIN/STDOUT/STDERR/etc)
const int32_t FD_NUM_RESERVED = 3;

//-----------------------------------------------------------------------------
// FileDescriptorTable Static Member Variables

volatile long						FileDescriptorTable::s_next = FD_NUM_RESERVED;
std::map<int32_t, FileDescriptor>	FileDescriptorTable::s_alive;
std::queue<int32_t>					FileDescriptorTable::s_dead;
ReaderWriterLock					FileDescriptorTable::s_lock;

//-----------------------------------------------------------------------------
// FileDescriptorTable::Allocate (static)
//
// Allocates a new file descriptor and associates it with an os handle
//
// Arguments:
//
//	object		- Object returned from call to remote services
//	handle		- Opened operating system handle (or INVALID_HANDLE_VALUE)

int32_t FileDescriptorTable::Allocate(const FsObject& object, HANDLE handle)
{
	int32_t					fd;					// Descriptor to return

	AutoWriterLock lock(s_lock);

	// Ressureect a dead file descriptor index when one is available, otherwise
	// generate a sequentially new index for this entry
	if(!s_dead.empty()) { fd = s_dead.front(); s_dead.pop(); }
	else fd = (s_next == INT32_MAX) ? -1 : s_next++;

	// Ensure that an index was successfully allocated
	if(fd < 0) return fd;

	// Insert a new FileDescriptor into the collection table
	s_alive.insert(std::make_pair(fd, FileDescriptor(object, handle)));

	return fd;
}

//-----------------------------------------------------------------------------
// FileDescriptorTable::Free (static)
//
// Releases a previously allocated file descriptor
//
// Arguments:
//
//	fd			- Spent file descriptor index

void FileDescriptorTable::Free(int32_t fd)
{
	AutoWriterLock lock(s_lock);

	s_alive.erase(fd);				// Remove from the active collection
	s_dead.push(fd);				// Push dead index into the queue
}

//-----------------------------------------------------------------------------
// FileDescriptorTable::Get (static)
//
// Accesses a file descriptor object
//
// Arguments:
//
//	fd			- File descriptor index

FileDescriptor FileDescriptorTable::Get(int32_t fd)
{
	AutoReaderLock lock(s_lock);

	std::map<int32_t, FileDescriptor>::iterator iterator;
	iterator = s_alive.find(fd);
	return (iterator == s_alive.end()) ? FileDescriptor::Null : iterator->second;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)