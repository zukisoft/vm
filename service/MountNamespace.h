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
#include <set>
#include <concrt.h>
#include "FileSystem.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// MountNamespace
//
// Provides an isolated view of file system mounts

class MountNamespace : public std::enable_shared_from_this<MountNamespace>
{
public:

	// Destructor
	//
	~MountNamespace()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new MountNamespace instance
	static std::shared_ptr<MountNamespace> Create(void);
	static std::shared_ptr<MountNamespace> Create(const std::shared_ptr<MountNamespace>& mountns);

private:

	MountNamespace(const MountNamespace&)=delete;
	MountNamespace& operator=(const MountNamespace&)=delete;

	// mount_set_t
	//
	// Collection type used to store the mounts for this namespace
	using mount_set_t = std::set<std::pair<std::unique_ptr<Mount>, std::shared_ptr<Alias>>>;

	// mount_set_lock_t
	//
	// Synchronization object used with mount_set_t collection
	using mount_set_lock_t = Concurrency::reader_writer_lock;

	// Instance Constructor
	//
	MountNamespace(mount_set_t&& mounts);
	friend class std::_Ref_count_obj<MountNamespace>;

	//-------------------------------------------------------------------------
	// Private Member Functions

	//-------------------------------------------------------------------------
	// Member Variables

	mount_set_t					m_mounts;			// Collection of mounts
	mount_set_lock_t			m_mountlock;		// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MOUNTNAMESPACE_H_
