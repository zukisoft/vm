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

//-----------------------------------------------------------------------------
// HostFileSystem
//
// todo: document
// todo: list mount options
// todo: list object lifetimes

class HostFileSystem : public FileSystem
{
public:

	// Destructor
	//
	virtual ~HostFileSystem()=default;

	// Mount
	//
	// Mounts the file system
	static NodePtr Mount(const tchar_t* device);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// Instance Constructor
	//
	HostFileSystem()=default;
	friend class std::_Ref_count_obj<HostFileSystem>;

	class File;
	class Node;

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
		// Creates a directory node as a child of this node on the file system
		virtual NodePtr CreateDirectory(const DirectoryEntryPtr& dentry, uapi::mode_t mode);

		// CreateSymbolicLink (FileSystem::Node)
		//
		// Creates a symbolic link node as a child of this node on the file system
		virtual NodePtr CreateSymbolicLink(const DirectoryEntryPtr& dentry, const tchar_t* target);

		// Index (FileSystem::Node)
		//
		// Gets the node index
		__declspec(property(get=getIndex)) uint32_t Index;
		virtual uint32_t getIndex(void) { return static_cast<uint32_t>(m_index); }

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
	// Specialization of HostFileSystem::Node for the mount point node.  Unlike Node,
	// this class maintains a strong reference to the HostFileSystem instance to keep
	// it alive as long as the file system is mounted
	class RootNode : public Node
	{
	public:

		// Constructor / Destructor
		//
		RootNode(const std::shared_ptr<HostFileSystem>& fs, NodeType type, HANDLE handle) : Node(fs, type, handle), m_fs(fs) {}
		virtual ~RootNode()=default;

	private:

		// m_fs
		//
		// Strong reference to the parent file system instance
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