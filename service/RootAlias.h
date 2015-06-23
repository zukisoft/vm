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

#ifndef __ROOTALIAS_H_
#define __ROOTALIAS_H_
#pragma once

#include <map>
#include <memory>
#include <stack>
#include <concrt.h>
#include "FileSystem.h"
#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootAlias
//
// Implements the virtual file system absolute root alias object; must be
// associated with a node at time of construction

class RootAlias : public FileSystem::Alias, public std::enable_shared_from_this<RootAlias>
{
public:

	// Destructor
	//
	~RootAlias()=default;

	// Create
	//
	// Constructs a new RootAlias instance
	static std::shared_ptr<FileSystem::Alias> Create(const std::shared_ptr<FileSystem::Node>& target);

	// Follow (FileSystem::Alias)
	//
	// Follows this alias to the file system node that it refers to
	virtual std::shared_ptr<FileSystem::Node> Follow(const std::shared_ptr<Namespace>& ns);

	// Mount (FileSystem::Alias)
	//
	// Adds a mountpoint node to this alias, obscuring any existing node in the same namespace
	virtual void Mount(const std::shared_ptr<Namespace>& ns, const std::shared_ptr<FileSystem::Mount>& mount);

	// Unmount (FileSystem::Alias)
	//
	// Removes a mountpoint node from this alias
	virtual void Unmount(const std::shared_ptr<Namespace>& ns);

	// getName (FileSystem::Alias)
	//
	// Gets the name associated with the alias
	virtual const char_t* getName(void);

	// getParent (FileSystem::Alias)
	//
	// Gets the parent alias of this alias instance, or nullptr if none exists
	virtual std::shared_ptr<Alias> getParent(void);

private:

	RootAlias(const RootAlias&)=delete;
	RootAlias& operator=(const RootAlias&)=delete;

	// mounts_t
	//
	// Collection that associates overmounts in a specific namespace
	using mounts_t = std::map<std::shared_ptr<Namespace>, std::stack<std::shared_ptr<FileSystem::Mount>>>;

	// mountslock_t
	//
	// Synchronization object for the overmounts collection
	using mountslock_t = Concurrency::reader_writer_lock;

	// Instance Constructor
	//
	RootAlias(const std::shared_ptr<FileSystem::Node>& target);
	friend class std::_Ref_count_obj<RootAlias>;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::shared_ptr<FileSystem::Node>	m_target;		// Target node
	mounts_t								m_mounts;		// Overmounts
	mountslock_t							m_mountslock;	// Synchronziation
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTALIAS_H_