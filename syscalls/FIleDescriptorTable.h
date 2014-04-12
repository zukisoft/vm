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

#ifndef __FILEDESCRIPTORTABLE_H_
#define __FILEDESCRIPTORTABLE_H_

#include <queue>						// Include STL queue<> template
#include <map>							// Include STL map<> template decls
#include "AutoReaderLock.h"				// Include AutoReaderLock declarations
#include "AutoWriterLock.h"				// Include AutoWriterLock declarations
#include "FileDescriptor.h"				// Include FileDescriptor declarations
#include "FsObject.h"					// Include FsObject class declarations
#include "ReaderWriterLock.h"			// Include ReaderWriterLock declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// FileDescriptorTable
//
// Implements the proces-wide file descriptor table.  File descriptors can be
// allocated against virtual objects in the remote services, or against
// physical file system resources.
//
// File descriptors are allocated sequentially and added into a queue when
// released.  This allows previously owned file descriptors to be reused
// without needing to search through the table to find an available slot

class FileDescriptorTable
{
public:

	// Destructor
	//
	~FileDescriptorTable();

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates a file descriptor
	static int32_t Allocate(const FsObject& object) { return Allocate(object, INVALID_HANDLE_VALUE); }
	static int32_t Allocate(const FsObject& object, HANDLE handle);

	// Free
	//
	// Releases a file descriptor
	static void Free(int32_t fd);

	static FileDescriptor Get(int32_t fd);
	FileDescriptor operator[](int32_t fd) { return Get(fd); }

	//-------------------------------------------------------------------------
	// Properties

private:

	FileDescriptorTable();
	FileDescriptorTable(const FileDescriptorTable& rhs);
	FileDescriptorTable& operator=(const FileDescriptorTable& rhs);

	//-------------------------------------------------------------------------
	// Member Variables

	volatile static long						s_next;		// Next sequential descriptor
	static std::map<int32_t, FileDescriptor>	s_alive;	// Alive descriptors
	static std::queue<int32_t>					s_dead;		// Dead descriptors
	static ReaderWriterLock						s_lock;		// Synchronization object
};


//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __FILEDESCRIPTORTABLE_H_
