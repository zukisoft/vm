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
#include "IndexPool.h"
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
	virtual ~HostFileSystem()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Mount (static)
	//
	// Mounts the file system
	static FileSystemPtr Mount(const tchar_t* device);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;
	
	// _indexpool_t
	using _indexpool_t = IndexPool<int32_t>;

	// Instance Constructor
	//
	HostFileSystem(const std::shared_ptr<_indexpool_t>& indexpool, const tchar_t* device);
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
		Node(const std::shared_ptr<_indexpool_t>& indexpool, NodeType type, HANDLE handle);
		virtual ~Node();

		// CreateDirectory (FileSystem::Node)
		//
		// Creates a directory node as a child of this node on the file system
		virtual NodePtr CreateDirectory(const tchar_t* name, uapi::mode_t mode);

		// CreateSymbolicLink (FileSystem::Node)
		//
		// Creates a symbolic link node as a child of this node on the file system
		virtual NodePtr CreateSymbolicLink(const tchar_t* name, const tchar_t* target);

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

		// m_indexpool
		//
		// Weak reference to the index pool
		std::weak_ptr<_indexpool_t> m_indexpool;

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

	// OpenHostDirectory
	//
	// Opens a directory object on the host file system
	static HANDLE OpenHostDirectory(const tchar_t* path);

	// OpenHostSymbolicLink
	//
	// Opens a symbolic link object on the host file system
	static HANDLE OpenHostSymbolicLink(const tchar_t* path);

	virtual NodePtr getRootNode(void) { return m_rootnode; }

	//-------------------------------------------------------------------------
	// Member Variables

	NodePtr m_rootnode;

	std::shared_ptr<_indexpool_t> m_indexpool;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_