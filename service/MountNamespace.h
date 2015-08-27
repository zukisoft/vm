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

#ifndef __MOUNTNAMESPACE_H_
#define __MOUNTNAMESPACE_H_
#pragma once

#include <memory>
#include <stack>
#include <unordered_map>
#include "FileSystem.h"
#include "LinuxException.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// MountNamespace
//
// Provides an isolated view of file system mounts

class MountNamespace
{
public:

	// Destructor
	//
	~MountNamespace()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Add
	//
	// Adds an alias as a mount point in this namespace
	void Add(std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount);

	// Clone
	//
	// Clones this MountNamespace instance
	std::shared_ptr<MountNamespace> Clone(void);

	// Create (static)
	//
	// Creates a new MountNamespace instance
	static std::shared_ptr<MountNamespace> Create(void);

	// Find
	//
	// Finds the mount point associated with an alias, or nullptr if none
	std::shared_ptr<FileSystem::Mount> Find(std::shared_ptr<const FileSystem::Alias> alias);

	// Remove
	//
	// Removes the topmost mount point associated with an alias instance
	void Remove(std::shared_ptr<const FileSystem::Alias> alias);

private:

	MountNamespace(const MountNamespace&)=delete;
	MountNamespace& operator=(const MountNamespace&)=delete;

	// mount_map_t
	//
	// Collection type used to store the mounts for this namespace
	using mount_map_t = std::unordered_map<std::shared_ptr<const FileSystem::Alias>, std::stack<std::shared_ptr<FileSystem::Mount>>>;
	
	// mount_map_lock_t
	//
	// Synchronization object used with mount_map_t collection
	using mount_map_lock_t = sync::reader_writer_lock;

	// Instance Constructor
	//
	MountNamespace(mount_map_t&& mounts);
	friend class std::_Ref_count_obj<MountNamespace>;

	//-------------------------------------------------------------------------
	// Member Variables

	mount_map_t					m_mounts;			// Collection of mounts
	mount_map_lock_t			m_mountslock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MOUNTNAMESPACE_H_
