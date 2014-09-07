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

#ifndef __ROOTFILESYSTEM_H_
#define __ROOTFILESYSTEM_H_
#pragma once

#include <memory>
#include <mutex>
#include <stack>
#include "LinuxException.h"
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem
//
// RootFileSystem implements a file system that contains one and only one node
// that does not support creation of any child objects.  The intent behind this
// file system is to provide a default master root that would be overmounted by
// another file system when the virtual machine is starting up if no other
// more robust default file system is available

class RootFileSystem : public FileSystem, public FileSystem::Directory, public FileSystem::Alias,
	public std::enable_shared_from_this<RootFileSystem>
{
public:

	// Destructor
	virtual ~RootFileSystem()=default;

	// Mount
	//
	// Mounts the file system
	static FileSystemPtr Mount(const tchar_t* source);

private:

	RootFileSystem(const RootFileSystem&)=delete;
	RootFileSystem& operator=(const RootFileSystem&)=delete;

	// Constructor
	//
	RootFileSystem()=default;
	friend class std::_Ref_count_obj<RootFileSystem>;

	// FileSystem Implementation
	//
	virtual AliasPtr				getRoot(void) { return shared_from_this(); }

	// FileSystem::Alias Implementation
	//
	virtual void					Mount(const FileSystem::NodePtr& node);
	virtual void					Unmount(void);	
	virtual const tchar_t*			getName(void) { return _T(""); }
	virtual FileSystem::NodePtr		getNode(void);
	virtual FileSystem::AliasPtr	getParent(void) { return shared_from_this(); }

	// FileSystem::Node Implementation
	//
	virtual FileSystem::HandlePtr	Open(int) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	virtual FileSystem::AliasPtr	Resolve(const FileSystem::AliasPtr&, const tchar_t* path, int);
	virtual uint64_t				getIndex(void) { return FileSystem::NODE_INDEX_ROOT; }
	virtual NodeType				getType(void) { return NodeType::Directory; }

	// FileSystem::Directory Implementation
	//
	virtual void					CreateDirectory(const FileSystem::AliasPtr&, const tchar_t*) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	virtual FileSystem::HandlePtr	CreateFile(const FileSystem::AliasPtr&, const tchar_t*, int) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
	virtual void					CreateSymbolicLink(const FileSystem::AliasPtr&, const tchar_t*, const tchar_t*) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); } 
	virtual void					RemoveNode(const tchar_t*) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }
		
	//-------------------------------------------------------------------------
	// Member Variables

	std::mutex							m_lock;		// Synchronization object
	std::stack<FileSystem::NodePtr>		m_mounted;	// Stack of mounted nodes
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_