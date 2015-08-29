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

#ifndef __PROCESSHANDLES_H_
#define __PROCESSHANDLES_H_
#pragma once

#include <memory>
#include <unordered_map>
#include "FileSystem.h"
#include "IndexPool.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcessHandles
//
// Implements a collection of file system handles that can be duplicated or
// shared among multiple process instances

class ProcessHandles
{
public:

	// Destructor
	//
	~ProcessHandles()=default;

	// Array subscript operator
	//
	std::shared_ptr<FileSystem::Handle> operator[](int fd) { return Get(fd); }

	//-------------------------------------------------------------------------
	// Member Functions

	// Add
	//
	// Adds a file system handle to the collection
	int Add(std::shared_ptr<FileSystem::Handle> handle);
	int Add(int fd, std::shared_ptr<FileSystem::Handle> handle);

	// Create (static)
	//
	// Creates a new empty handle collection
	static std::shared_ptr<ProcessHandles> Create(void);

	// Duplicate (static)
	//
	// Duplicates the handle collection into a new instance
	static std::shared_ptr<ProcessHandles> Duplicate(std::shared_ptr<ProcessHandles> existing);

	// Get
	//
	// Accesses a file system handle by the file descriptor
	std::shared_ptr<FileSystem::Handle> Get(int fd);

	// Remove
	//
	// Removes a file system handle from the collection
	void Remove(int fd);

	// RemoveCloseOnExecute
	//
	// Releases all file system handles set for close-on-exec
	void RemoveCloseOnExecute(void);

private:

	ProcessHandles(const ProcessHandles&)=delete;
	ProcessHandles& operator=(const ProcessHandles&)=delete;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// MIN_FD_INDEX
	//
	// Minimum allowable file descriptor index
	static const int MIN_FD_INDEX = 3;

	// handle_lock_t
	//
	// File system handle collection synchronization object
	using handle_lock_t = sync::reader_writer_lock;
	
	// handle_map_t
	//
	// Collection of file system handles, keyed on the index (file descriptor)
	using handle_map_t = std::unordered_map<int, std::shared_ptr<FileSystem::Handle>>;

	//-------------------------------------------------------------------------
	// Instance Constructors
	//
	ProcessHandles();
	ProcessHandles(handle_map_t&& handles, IndexPool<int>&& fdpool);
	friend class std::_Ref_count_obj<ProcessHandles>;

	//-------------------------------------------------------------------------
	// Member Variables

	handle_lock_t			m_handlelock;		// Handle collection lock
	handle_map_t			m_handles;			// Process file system handles
	IndexPool<int>			m_fdpool;			// File descriptor index pool
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCESSHANDLES_H_
