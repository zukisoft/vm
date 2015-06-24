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
#include "RootAlias.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootAlias Constructor (private)
//
// Arguments:
//
//	ns			- Namespace in which the initial mount is a member
//	mount		- Reference to the initial mountpoint

RootAlias::RootAlias(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<FileSystem::Mount>& mount)
{
	_ASSERTE(ns);
	_ASSERTE(mount);

	// Add the initial namespace/mount combination to the collection
	try { m_mounts[ns].push(mount); }
	catch(...) { throw LinuxException(LINUX_ENOMEM); }
}

//-----------------------------------------------------------------------------
// RootAlias::Create (static)
//
// Creates an instance of the root alias object
//
// Arguments:
//
//	ns			- Namespace in which the initial mount is a member
//	mount		- Reference to the initial mountpoint

std::shared_ptr<FileSystem::Alias> RootAlias::Create(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<FileSystem::Mount>& mount)
{
	return std::make_shared<RootAlias>(ns, mount);
}

//-----------------------------------------------------------------------------
// RootAlias::Follow
//
// Follows this alias to the node that it refers to in the specified namespace
//
// Arguments:
//
//	ns		- Namespace in which to resolve the alias

std::shared_ptr<FileSystem::Node> RootAlias::Follow(const std::shared_ptr<Namespace>& ns)
{
	mountslock_t::scoped_lock_read reader(m_mountslock);

	// If this namespace has been overmounted, return the topmost mount root node
	const auto& iterator = m_mounts.find(ns);
	if(iterator != m_mounts.end()) return iterator->second.top()->Root;

	// The root alias must have a mount entry in every namespace
	throw LinuxException(LINUX_ENOENT);
}

//-----------------------------------------------------------------------------
// RootAlias::PopMount
//
// Removes the topmost overmount within the specified namespace
//
// Arguments:
//
//	ns		- Namespace in which to remove the topmost mount

void RootAlias::PopMount(const std::shared_ptr<Namespace>& ns)
{
	mountslock_t::scoped_lock writer(m_mountslock);

	// Locate the target namespace entry in the overmounts collection
	const auto& iterator = m_mounts.find(ns);
	if(iterator != m_mounts.end()) {

		// The stack instance in the collection should never be empty, the entire
		// namespace key should have been removed from the collection (see below)
		_ASSERTE(!iterator->second.empty());

		// Remove the topmost mount instance from the overmount stack, if that
		// reduces the size of the stack to zero, remove the entire entry
		iterator->second.pop();
		if(iterator->second.empty()) m_mounts.erase(iterator);
	}
}

//-----------------------------------------------------------------------------
// RootAlias::PushMount
//
// Overmounts this alias such that within the specified namespace it will refer
// to the root node of a mount point rather than the directly associated node
//
// Arguments:
//
//	ns		- Namespace in which to register the mount
//	mount	- Mount instance to be registered as the target for this alias

void RootAlias::PushMount(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<FileSystem::Mount>& mount)
{
	mountslock_t::scoped_lock writer(m_mountslock);

	try { m_mounts[ns].push(mount); }
	catch(...) { throw LinuxException(LINUX_ENOMEM); }
}

//-----------------------------------------------------------------------------
// RootAlias::getMount
//
// Retrieves the mountpoint instance for the specified namespace, or nullptr if not mounted

std::shared_ptr<FileSystem::Mount> RootAlias::getMount(const std::shared_ptr<Namespace>& ns)
{
	mountslock_t::scoped_lock_read reader(m_mountslock);

	// Locate the target namespace entry in the overmounts collection and
	// return a shared pointer to the topmost mount instance
	const auto& iterator = m_mounts.find(ns);
	return (iterator != m_mounts.end()) ? iterator->second.top() : nullptr;
}
//-----------------------------------------------------------------------------
// RootAlias::getMounted
//
// Determines if the alias serves as a mountpoint in the specified namespace

bool RootAlias::getMounted(const std::shared_ptr<Namespace>& ns)
{
	mountslock_t::scoped_lock_read reader(m_mountslock);

	// Locate the target namespace entry in the overmounts collection
	const auto& iterator = m_mounts.find(ns);
	return iterator != m_mounts.end();
}

//-----------------------------------------------------------------------------
// RootAlias::getName
//
// Gets the name associated with this alias

const char_t* RootAlias::getName(void)
{
	return "";
}

//-----------------------------------------------------------------------------
// RootAlias::getParent
//
// Gets a reference to the parent alias

std::shared_ptr<FileSystem::Alias> RootAlias::getParent(void)
{
	return shared_from_this();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
