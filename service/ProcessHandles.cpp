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

#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessHandles Constructor (private)
//
// Arguments:
//
//	NONE

ProcessHandles::ProcessHandles() : m_fdpool{ MIN_FD_INDEX } 
{
}

//-----------------------------------------------------------------------------
// ProcessHandles Constructor (private)
//
// Arguments:
//
//	handles		- Existing collection of handles to use
//	fdpool		- Existing pool of handle indices to use

ProcessHandles::ProcessHandles(handle_map_t&& handles, IndexPool<int>&& fdpool) : m_handles{ std::move(handles) }, m_fdpool{ std::move(fdpool) } 
{
}

//-----------------------------------------------------------------------------
// ProcessHandles::Add
//
// Adds a file system handle to the collection
//
// Arguments:
//
//	handle		- Handle instance to be added to the process

int ProcessHandles::Add(std::shared_ptr<FileSystem::Handle> handle)
{
	handle_lock_t::scoped_lock_write writer(m_handlelock);

	// Allocate a new file descriptor for the handle and insert it
	int index = m_fdpool.Allocate();
	if(m_handles.emplace(index, std::move(handle)).second) return index;

	// Insertion failed, release the index back to the pool and throw
	m_fdpool.Release(index);
	throw LinuxException{ LINUX_EBADF };
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

int ProcessHandles::Add(int fd, std::shared_ptr<FileSystem::Handle> handle)
{
	handle_lock_t::scoped_lock_write writer(m_handlelock);

	// Attempt to insert the handle using the specified index
	if(m_handles.emplace(fd, std::move(handle)).second) return fd;
	throw LinuxException{ LINUX_EBADF };
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
	return std::make_shared<ProcessHandles>();
}

//-----------------------------------------------------------------------------
// ProcessHandles::Duplicate (static)
//
// Duplicates an existing handle collection
//
// Arguments:
//
//	existing		- The existing collection instance to be duplicated

std::shared_ptr<ProcessHandles> ProcessHandles::Duplicate(std::shared_ptr<ProcessHandles> existing)
{
	handle_map_t		handles;						// New collection for the duplicate handles
	IndexPool<int>		fdpool(existing->m_fdpool);		// Index pool for the new collection

	// The existing collection needs to be locked for read access during duplication
	handle_lock_t::scoped_lock_read reader(existing->m_handlelock);

	// Iterate over the existing collection and duplicate each handle with the same flags
	for(const auto& iterator : existing->m_handles)
		if(!handles.emplace(iterator.first, std::move(iterator.second->Duplicate())).second) throw LinuxException{ LINUX_EBADF };

	// Create the ProcessHandles instance with the duplicated collection
	return std::make_shared<ProcessHandles>(std::move(handles), std::move(fdpool));
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
	catch(...) { throw LinuxException{ LINUX_EBADF }; }
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
	handle_lock_t::scoped_lock_write writer(m_handlelock);

	// Remove the index from the collection and return it to the pool
	if(m_handles.erase(fd) == 0) throw LinuxException{ LINUX_EBADF };
	m_fdpool.Release(fd);
}

//-----------------------------------------------------------------------------
// ProcessHandles::RemoveCloseOnExecute
//
// Closes all handles that are marked as close-on-exec
//
// Arguments:
//
//	NONE

void ProcessHandles::RemoveCloseOnExecute(void)
{
	handle_lock_t::scoped_lock_write writer(m_handlelock);

	// Iterate over all the contained file system handles to look for CLOEXEC
	auto iterator = m_handles.begin();
	while(iterator != m_handles.end()) {

		// If the file handle is set to be closed on execute, remove it
		// from the member collection
		if(iterator->second->CloseOnExec) iterator = m_handles.erase(iterator);
		else ++iterator;
	}
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
