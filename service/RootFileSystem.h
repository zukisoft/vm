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
// RootFileSystem implements the specialized file system that serves as the 
// root node for everything else.  All that exists is the root Alias that allows
// for another file system to be mounted on top of it.

class RootFileSystem : public FileSystem, public FileSystem::Alias, public FileSystem::Node,
	public std::enable_shared_from_this<RootFileSystem>
{
public:

	// Destructor
	virtual ~RootFileSystem()=default;

	// Mount
	//
	// Mounts the file system
	// TODO: should be something else or return NodePtr, perhaps NodePtr
	// will work, just need a special alias to wrap it in the service?
	static AliasPtr Mount(const tchar_t* device);

private:

	RootFileSystem(const RootFileSystem&)=delete;
	RootFileSystem& operator=(const RootFileSystem&)=delete;

	// Instance Constructor
	//
	RootFileSystem()=default;
	friend class std::_Ref_count_obj<RootFileSystem>;

	// Mount (FileSystem::Alias)
	//
	// Mounts (pushes) a node instance to this alias
	virtual void Mount(const NodePtr& node);

	// Unmount (FileSystem::Alias)
	//
	// Unmounts (pops) a node instance from this alias
	virtual void Unmount(void);

	// Index (FileSystem::Node)
	//
	// Gets the node index
	__declspec(property(get=getIndex)) uint32_t Index;
	virtual uint32_t getIndex(void) { return 0; }

	// MountPoint
	//
	// Determines if this alias is acting as a mount point
	__declspec(property(get=getMountPoint)) bool MountPoint;
	virtual bool getMountPoint(void);

	// Name (FileSystem::Alias)
	//
	// Gets the name assigned to this alias instance
	__declspec(property(get=getName)) const tchar_t* Name;
	virtual const tchar_t* getName(void) { return _T("/"); }

	// Node (FileSystem::Alias)
	//
	// Gets the node attached to this alias, or nullptr if not attached
	__declspec(property(get=getNode)) NodePtr Node;
	virtual NodePtr getNode(void);

	// State (FileSystem::Alias)
	//
	// Gets the state (attached/detached) of this alias instance
	__declspec(property(get=getState)) AliasState State;
	virtual AliasState getState(void) { return AliasState::Attached; }

	// Type (FileSystem::Node)
	//
	// Gets the node type
	__declspec(property(get=getType)) NodeType Type;
	virtual NodeType getType(void) { return NodeType::Directory; }

	// m_lock
	//
	// Mutex to control access to the nodes collection
	std::mutex m_lock;

	// m_nodes
	//
	// Collection of nodes attached to this Alias instance
	std::stack<NodePtr> m_nodes;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_