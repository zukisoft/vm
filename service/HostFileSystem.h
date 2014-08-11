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

#ifndef __HOSTFILESYSTEM_H_
#define __HOSTFILESYSTEM_H_
#pragma once

// todo: not using all of these
#include <atomic>
#include <concurrent_queue.h>
#include <concurrent_vector.h>
#include <memory>
#include <mutex>
#include <stack>
#include <linux/stat.h>
#include "LinuxException.h"
#include "Win32Exception.h"

#include "FileSystem.h"

#pragma warning(push, 4)

// Special alias to be incorporated into the service
// move to RootFileSystem.h/cpp
// Supports overmounting - yes

class VmRootAlias : public FileSystem::Alias
{
public:

	VmRootAlias()=default;
	~VmRootAlias()=default;

	virtual void PushNode(const std::shared_ptr<FileSystem::Node>& node)
	{
		std::lock_guard<std::mutex> critsec(m_nodelock);

		// Push the node into the stack, this will mask any existing node
		m_nodes.push(node);
	}

	virtual std::shared_ptr<FileSystem::Node> PopNode(void)
	{
		std::lock_guard<std::mutex> critsec(m_nodelock);

		if(m_nodes.empty()) return nullptr; 

		// Pull the top shared_ptr<> from the stack and pop it
		auto result = m_nodes.top();
		m_nodes.pop();
		return result;
	}

	// name is always blank for the root alias
	virtual const tchar_t* getName(void) { return _T(""); }

	virtual std::shared_ptr<FileSystem::Node> getNode(void) 
	{
		std::lock_guard<std::mutex> critsec(m_nodelock);

		// Return an empty shared_ptr<> if there is no attached node
		return (m_nodes.empty()) ? nullptr : m_nodes.top();
	}

	// Parent
	//
	// Gets the parent alias for this alias instance
	__declspec(property(get=getParent)) std::shared_ptr<FileSystem::Alias> Parent;
	virtual std::shared_ptr<FileSystem::Alias> getParent(void) { return nullptr; }

	// State
	//
	// Gets the state (attached/detached) of this alias instance
	__declspec(property(get=getState)) FileSystem::AliasState State;
	virtual FileSystem::AliasState getState(void) { 
		
		std::lock_guard<std::mutex> critsec(m_nodelock);
		return m_nodes.size() == 0 ? FileSystem::AliasState::Detached : FileSystem::AliasState::Attached; 
	}

private:

	VmRootAlias(const VmRootAlias&)=delete;
	VmRootAlias& operator=(const VmRootAlias&)=delete;

	std::stack<std::shared_ptr<FileSystem::Node>> m_nodes;
	std::mutex m_nodelock;
};
/// END: MOVE ME

//-----------------------------------------------------------------------------
// HostFileSystem
//
// todo: document
// todo: list mount options
// todo: list object lifetimes

class HostFileSystem : public FileSystem
{
public:

	// Constructor / Destructor
	//
	HostFileSystem()=default;
	virtual ~HostFileSystem()=default;

	// Mount
	//
	// Mounts the file system
	static NodePtr Mount(const tchar_t* device);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	class Alias;
	class File;
	class Node;

	// Alias
	//
	// FileSystem::Alias implementation
	class Alias : public FileSystem::Alias
	{
	public:

		// Instance Constructors
		//
		Alias(const std::shared_ptr<FileSystem::Alias>& parent, const tchar_t* name) : m_parent(parent), m_name(name) {}
		Alias(const std::shared_ptr<FileSystem::Alias>& parent, const tchar_t* name, const std::shared_ptr<Node>& node) : m_parent(parent), m_name(name), m_node(node) {}

		// Destructor
		//
		virtual ~Alias()=default;

		// Name (FileSystem::Alias)
		//
		// Gets the name assigned to this alias instance
		__declspec(property(get=getName)) const tchar_t* Name;
		virtual const tchar_t* getName(void) const { return m_name.c_str(); }

		// Parent (FileSystem::Alias)
		//
		// Gets the parent alias for this alias instance
		__declspec(property(get=getParent)) std::shared_ptr<FileSystem::Alias> Parent;
		virtual std::shared_ptr<FileSystem::Alias> getParent(void) const { return nullptr; }

		// State (FileSystem::Alias)
		//
		// Gets the state (attached/detached) of this alias instance
		__declspec(property(get=getState)) FileSystem::AliasState State;
		virtual FileSystem::AliasState getState(void) const { return (m_node) ? AliasState::Attached : AliasState::Detached; }

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		// m_name
		//
		// The name assigned to this alias
		std::tstring m_name;

		// m_node
		//
		// The node that is attached to this alias
		std::shared_ptr<Node> m_node;

		// m_parent
		//
		// Strong reference to this alias' parent alias
		std::shared_ptr<FileSystem::Alias> m_parent;
	};

	// File
	//
	// FileSystem::File implementation
	class File : public FileSystem::File
	{
	public:

		// Destructor
		//
		virtual ~File()=default;

	private:

		File(const File&)=delete;
		File& operator=(const File&)=delete;
	};

	// HostFileSystem::Node
	//
	// Specialization of FileSystem::Node for a host file system instance
	class Node : public FileSystem::Node
	{
	public:

		// Constructor / Destructor
		//
		Node(const std::shared_ptr<HostFileSystem>& fs, NodeType type, HANDLE handle);
		virtual ~Node();

		// CreateDirectory (FileSystem::Node)
		//
		// Creates a directory as a child of this node
		virtual NodePtr CreateDirectory(const tchar_t* name, uapi::mode_t mode);

		// CreateSymbolicLink (FileSystem::Node)
		//
		// Creates a symbolic link node as a child of this node instance
		virtual NodePtr CreateSymbolicLink(const tchar_t* name, const tchar_t* target);

		// Index (FileSystem::Node)
		//
		// Gets the node index
		__declspec(property(get=getIndex)) uint32_t Index;
		virtual uint32_t getIndex(void) { return static_cast<uint32_t>(m_index); }

		// MountPoint (FileSystem::Node)
		//
		// Indicates if this node is a mount point
		__declspec(property(get=getMountPoint)) bool MountPoint;
		virtual bool getMountPoint(void) { return false; }

		// Type (FileSystem::Node)
		//
		// Gets the node type
		__declspec(property(get=getType)) NodeType Type;
		virtual NodeType getType(void) { return m_type; }

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		// m_fs
		//
		// Weak reference to the parent file system object
		std::weak_ptr<HostFileSystem> m_fs;

		// m_handle
		//
		// The underlying operating system object handle
		HANDLE m_handle = INVALID_HANDLE_VALUE;

		// m_index
		//
		// The allocated node index for this object
		int32_t m_index = -1;

		// m_type
		//
		// The type of node being represented
		const NodeType m_type;
	};

	// HostFileSystem::RootNode
	//
	// Specialization of HostFileSystem::Node for the mount point node
	class RootNode : public Node
	{
	public:

		// Constructor / Destructor
		//
		RootNode(const std::shared_ptr<HostFileSystem>& fs, NodeType type, HANDLE handle) : Node(fs, type, handle), m_fs(fs) {}
		virtual ~RootNode()=default;
		
		// MountPoint (FileSystem::Node)
		//
		// Indicates if this node is a mount point
		__declspec(property(get=getMountPoint)) bool MountPoint;
		virtual bool getMountPoint(void) { return true; }

	private:

		// m_fs
		//
		// Strong reference to the parent file system instance, this keeps the
		// parent HostFileSystem alive as long as the root node exists
		std::shared_ptr<HostFileSystem> m_fs;
	};

	// AllocateNodeIndex
	//
	// Allocates a node index from the pool
	int32_t AllocateNodeIndex(void);

	// OpenHostDirectory
	//
	// Opens a directory object on the host file system
	static HANDLE OpenHostDirectory(const tchar_t* path);

	// OpenHostSymbolicLink
	//
	// Opens a symbolic link object on the host file system
	static HANDLE OpenHostSymbolicLink(const tchar_t* path);

	// ReleaseNodeIndex
	//
	// Releases a node index from the pool
	void ReleaseNodeIndex(int32_t index);

	// m_nextindex
	//
	// Next sequential node index value
	std::atomic<int32_t> m_nextindex = 0;

	// m_spentindexes
	//
	// Queue used to recycle node indexes
	Concurrency::concurrent_queue<int32_t> m_spentindexes;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_