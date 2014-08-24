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

class RootFileSystem : public FileSystem, public FileSystem::Node, public FileSystem::Alias,
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

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// getRoot
	//
	// Gets the root alias instance
	virtual AliasPtr getRoot(void) { return shared_from_this(); }

	//-------------------------------------------------------------------------
	// FileSystem::Alias Implementation

	// Mount
	//
	// Mounts/binds a foreign node to this alias, obscuring the previous node
	virtual void Mount(const FileSystem::NodePtr& node);

	// Unmount
	//
	// Unmounts/unbinds a node from this alias, revealing the previously bound node
	virtual void Unmount(void);	
	
	// getName
	//
	// Gets the name associated with this alias
	virtual const tchar_t* getName(void) { return _T(""); }

	// getNode
	//
	// Accesses the node pointed to by this alias
	virtual FileSystem::NodePtr getNode(void);

	//-------------------------------------------------------------------------
	// FileSystem::Node Implementation

	// CreateDirectory
	//
	// Creates a directory node as a child of this node
	virtual void CreateDirectory(const tchar_t*)
	{ 
		throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); 
	}

	// CreateSymbolicLink
	//
	// Creates a new symbolic link as a child of this node
	virtual void CreateSymbolicLink(const tchar_t*, const tchar_t*)
	{ 
		throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); 
	}
	
	// OpenHandle
	//
	// Creates a FileSystem::Handle instance for this node on the specified alias
	virtual FileSystem::HandlePtr OpenHandle(const FileSystem::AliasPtr&, int)
	{ 
		throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); 
	}
		
	// ResolvePath
	//
	// Resolves a path for an alias that is a child of this alias
	virtual FileSystem::AliasPtr ResolvePath(const tchar_t* path, bool follow);

	// getIndex
	//
	// Gets the node index
	virtual uapi::ino_t getIndex(void) { return FileSystem::NODE_INDEX_ROOT; }

	// getType
	//
	// Gets the node type
	virtual NodeType getType(void) { return NodeType::Directory; }

	//-------------------------------------------------------------------------
	// Member Variables

	std::mutex							m_lock;		// Synchronization object
	std::stack<FileSystem::NodePtr>		m_mounted;	// Stack of mounted nodes
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_