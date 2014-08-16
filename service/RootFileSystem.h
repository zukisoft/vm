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
#include <stack>
#include <linux/stat.h>
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

class RootFileSystem : public FileSystem, public FileSystem::Node,
	public std::enable_shared_from_this<RootFileSystem>
{
public:

	// Destructor
	virtual ~RootFileSystem()=default;

	// Mount
	//
	// Mounts the file system
	// + unsigned long flags
	// + const void* data
	static FileSystemPtr Mount(const tchar_t* device /*todo: mount options -- must be RO*/);

private:

	RootFileSystem(const RootFileSystem&)=delete;
	RootFileSystem& operator=(const RootFileSystem&)=delete;

	// Instance Constructor
	RootFileSystem()=default;
	friend class std::_Ref_count_obj<RootFileSystem>;

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// RootNode
	//
	// Gets the root node instance
	__declspec(property(get=getRootNode)) NodePtr RootNode;
	virtual NodePtr getRootNode(void) { return shared_from_this(); }

	//-------------------------------------------------------------------------
	// FileSystem::Node Implementation

	// CreateDirectory
	//
	// Creates a directory node as a child of this node on the file system
	virtual NodePtr CreateDirectory(const tchar_t*, uapi::mode_t) { throw LinuxException(LINUX_EPERM); }

	// CreateSymbolicLink
	//
	// Creates a symbolic link node as a child of this node on the file system
	virtual NodePtr CreateSymbolicLink(const tchar_t*, const tchar_t*) { throw LinuxException(LINUX_EPERM); }

	// Index
	//
	// Gets the node index
	__declspec(property(get=getIndex)) uint32_t Index;
	virtual uint32_t getIndex(void) { return 0; }

	// Type
	//
	// Gets the node type
	__declspec(property(get=getType)) NodeType Type;
	virtual NodeType getType(void) { return NodeType::Directory; }
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_