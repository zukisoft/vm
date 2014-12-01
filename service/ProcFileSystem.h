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

#ifndef __PROCFILESYSTEM_H_
#define __PROCFILESYSTEM_H_
#pragma once

#include <memory>
#include <mutex>
#include <stack>
#include <linux/errno.h>
#include "FileSystem.h"
#include "LinuxException.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ProcFileSystem
//
// todo: words

class ProcFileSystem : public FileSystem, public FileSystem::Directory, public FileSystem::Alias,
	public std::enable_shared_from_this<ProcFileSystem>
{
public:

	// Destructor
	virtual ~ProcFileSystem()=default;

	// Create
	//
	// Creates an instance of the file system
	static FileSystemPtr Create(void);

private:

	ProcFileSystem(const ProcFileSystem&)=delete;
	ProcFileSystem& operator=(const ProcFileSystem&)=delete;

	// Constructor
	//
	ProcFileSystem()=default;
	friend class std::_Ref_count_obj<ProcFileSystem>;

	// FileSystem Implementation
	//
	virtual AliasPtr				getRoot(void) { return shared_from_this(); }

	// FileSystem::Alias Implementation
	//
	virtual void					Mount(const FileSystem::NodePtr& node);
	virtual void					Unmount(void);	
	virtual const uapi::char_t*		getName(void) { return ""; }
	virtual FileSystem::NodePtr		getNode(void);
	virtual FileSystem::AliasPtr	getParent(void) { return shared_from_this(); }

	// FileSystem::Node Implementation
	//
	virtual void					DemandPermission(uapi::mode_t) { throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL)); }
	virtual FileSystem::HandlePtr	Open(const AliasPtr&, int) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	virtual FileSystem::AliasPtr	Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks);
	//virtual uint64_t				getIndex(void) { return FileSystem::NODE_INDEX_ROOT; }
	virtual uapi::stat				getStatus(void);
	virtual NodeType				getType(void) { return NodeType::Directory; }

	// FileSystem::Directory Implementation
	//
	virtual void					CreateCharacterDevice(const AliasPtr&, const uapi::char_t*, uapi::mode_t, uapi::dev_t) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	virtual void					CreateDirectory(const FileSystem::AliasPtr&, const uapi::char_t*, uapi::mode_t) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	virtual FileSystem::HandlePtr	CreateFile(const FileSystem::AliasPtr&, const uapi::char_t*, int, uapi::mode_t) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	virtual void					CreateSymbolicLink(const FileSystem::AliasPtr&, const uapi::char_t*, const uapi::char_t*) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); } 
		
	//-------------------------------------------------------------------------
	// Member Variables

	std::mutex							m_lock;		// Synchronization object
	std::stack<FileSystem::NodePtr>		m_mounted;	// Stack of mounted nodes
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __PROCFILESYSTEM_H_