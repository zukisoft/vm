//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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
#include "ProcessHandles.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessHandles::Add
//
// Adds a file system handle to the collection
//
// Arguments:
//
//	handle		- Handle instance to be added to the process

int ProcessHandles::Add(const std::shared_ptr<FileSystem::Handle>& handle)
{
	handle_lock_t::scoped_lock writer(m_handlelock);

	// Allocate a new file descriptor for the handle and insert it
	int index = m_fdpool.Allocate();
	if(m_handles.insert(std::make_pair(index, handle)).second) return index;

	// Insertion failed, release the index back to the pool and throw
	m_fdpool.Release(index);
	throw LinuxException(LINUX_EBADF);
}

//-----------------------------------------------------------------------------
// ProcessHandles::Add
//
// Adds a file system handle to the collection with a specific file descriptor
//
// Arguments:
//
//	fd			- Specific file descriptor index to use
//	handle		- Handle instance to be added to the process

int ProcessHandles::Add(int fd, const FileSystem::HandlePtr& handle)
{
	handle_lock_t::scoped_lock writer(m_handlelock);

	// Attempt to insert the handle using the specified index
	if(m_handles.insert(std::make_pair(fd, handle)).second) return fd;
	throw LinuxException(LINUX_EBADF);
}

//-----------------------------------------------------------------------------
// ProcessHandles::Create (static)
//
// Creates a new empty handle collection instance
//
// Arguments:
//
//	NONE

std::shared_ptr<ProcessHandles> ProcessHandles::Create(void)
{
	// Create the ProcessHandles instance with a new empty collection
	return std::make_shared<ProcessHandles>(std::move(handle_map_t()));
}

//-----------------------------------------------------------------------------
// ProcessHandles::Duplicate (static)
//
// Duplicates an existing handle collection
//
// Arguments:
//
//	existing		- The existing collection instance to be duplicated

std::shared_ptr<ProcessHandles> ProcessHandles::Duplicate(const std::shared_ptr<ProcessHandles>& existing)
{
	handle_map_t			handles;			// New collection for the duplicate handles

	// Iterate over the existing collection and duplicate each handle with the same flags
	for(auto iterator : existing->m_handles)
		if(!handles.insert(std::make_pair(iterator.first, iterator.second->Duplicate(iterator.second->Flags))).second) throw LinuxException(LINUX_EBADF);

	// Create the ProcessHandles instance with the duplicated collection
	return std::make_shared<ProcessHandles>(std::move(handles));
}

//-----------------------------------------------------------------------------
// ProcessHandles::Get
//
// Accesses a file system handle by its file descriptor index
//
// Arguments:
//
//	fd			- File descriptor of the target handle

std::shared_ptr<FileSystem::Handle> ProcessHandles::Get(int fd)
{
	handle_lock_t::scoped_lock_read reader(m_handlelock);

	try { return m_handles.at(fd); }
	catch(...) { throw LinuxException(LINUX_EBADF); }
}

//-----------------------------------------------------------------------------
// ProcessHandles::Remove
//
// Removes a file system handle from the collection
//
// Arguments:
//
//	fd		- File descriptor of the target handle

void ProcessHandles::Remove(int fd)
{
	handle_lock_t::scoped_lock writer(m_handlelock);

	// Remove the index from the collection and return it to the pool
	if(m_handles.erase(fd) == 0) throw LinuxException(LINUX_EBADF);
	m_fdpool.Release(fd);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
