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
#include "MountNamespace.h"

#pragma warning(push, 4)
#pragma warning(disable:4127)		// conditional expression is constant

//-----------------------------------------------------------------------------
// MountNamespace Constructor (private)
//
// Arguments:
//
//	mounts		- Collection of mounts to contain upon construction

MountNamespace::MountNamespace(mount_map_t&& mounts) : m_mounts(std::move(mounts)) 
{
}

//-----------------------------------------------------------------------------
// MountNamespace::Add
//
// Adds an alias as a mount point in this namespace
//
// Arguments:
//
//	alias		- Alias instance associated with the mount point
//	mount		- Mount instance to add to the namespace

void MountNamespace::Add(std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount)
{
	mount_map_lock_t::scoped_lock_write writer(m_mountslock);

	// Push the mount to the top of the stack associated with this alias
	try { m_mounts[alias].emplace(std::move(mount)); }
	catch(...) { throw LinuxException(LINUX_ENOMEM); }
}

//-----------------------------------------------------------------------------
// MountNamespace::Clone
//
// Creates a clone of the MountNamespace instance
//
// Arguments:
//
//	mountns		- Existing MountNamespace to be copied

std::shared_ptr<MountNamespace> MountNamespace::Clone(void)
{
	mount_map_lock_t::scoped_lock_write writer(m_mountslock);

	// Create a copy of the contained mounts collection for the new namespace
	return std::make_shared<MountNamespace>(mount_map_t(m_mounts));
}

//-----------------------------------------------------------------------------
// MountNamespace::Create (static)
//
// Creates a new MountNamespace instance
//
// Arguments:
//
//	NONE

std::shared_ptr<MountNamespace> MountNamespace::Create(void)
{
	// Create a new empty MountNamespace instance
	return std::make_shared<MountNamespace>(mount_map_t());
}

//-----------------------------------------------------------------------------
// MountNamespace::Find
//
// Finds the mount point associated with an alias, or nullptr if none
//
// Arguments:
//
//	alias		- Alias instance to look up in the mounts collection

std::shared_ptr<FileSystem::Mount> MountNamespace::Find(std::shared_ptr<const FileSystem::Alias> alias)
{
	mount_map_lock_t::scoped_lock_read reader(m_mountslock);

	// Check if this alias is a mount point, and if so return the topmost mount
	const auto& iterator = m_mounts.find(alias);
	return (iterator == m_mounts.end()) ? nullptr : iterator->second.top();
}

//-----------------------------------------------------------------------------
// MountNamespace::Remove
//
// Removes the topmost mount point associated with an alias instance
//
// Arguments:
//
//	alias		- Alias instance from which to remove the top mount point

void MountNamespace::Remove(std::shared_ptr<const FileSystem::Alias> alias)
{
	mount_map_lock_t::scoped_lock_write writer(m_mountslock);

	// Locate the target alias in the collection
	const auto& iterator = m_mounts.find(alias);
	if(iterator != m_mounts.end()) {

		// The stack instance in the collection should never be empty, the entire
		// thing should have been removed from the collection
		_ASSERTE(!iterator->second.empty());

		// Remove the topmost mount instance from the stack, if that reduces
		// the size of the stack to zero, remove the entire entry
		iterator->second.pop();
		if(iterator->second.empty()) m_mounts.erase(iterator);
	}
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
