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

#include <memory>
#include <Shlwapi.h>
#include <PathCch.h>
#include <linux/stat.h>
#include "LinuxException.h"
#include "Win32Exception.h"
#include "FileSystem.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "pathcch.lib")

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// HostFileSystem
//
// todo: document
//
// todo: use something else for inode numbers than the pool; removed for now
// could do something like hashing the path with the instance address, or get
// fancy and have a static allocator that works across instances (yes?)
//
// todo: note that this does not support overmounting within the file system (yet?)

class HostFileSystem : public FileSystem
{
public:

	// Destructor
	//
	virtual ~HostFileSystem()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Mount
	//
	// Mounts the file system
	static FileSystemPtr Mount(const tchar_t* source, uint32_t flags, void* data);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// SuperBlock
	//
	// Internal state and metadata about the mounted file system, all file
	// system objects will hold a reference to this object
	class SuperBlock
	{
	public:

		// Constructor / Destructor
		//
		SuperBlock(const std::vector<tchar_t>& path) : m_path(path) {}
		~SuperBlock()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// ValidateHandle
		//
		// Verifies that a newly opened handle meets all mount criteria
		void ValidateHandle(HANDLE handle);

	private:

		SuperBlock(const SuperBlock&)=delete;
		SuperBlock& operator=(const SuperBlock&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		bool					m_verifypath = true;	// Flag to verify the path
		std::vector<tchar_t>	m_path;					// Path to the mount point
	};

	// Node
	//
	// Specialization of FileSystem::Node for a host file system instance
	class Node : public FileSystem::Node, public FileSystem::Alias, public std::enable_shared_from_this<Node>
	{
	public:

		// Constructor / Destructor
		//
		Node(const std::shared_ptr<SuperBlock>& superblock, std::vector<tchar_t>&& path, FileSystem::NodeType type, HANDLE handle);
		virtual ~Node();

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		//---------------------------------------------------------------------
		// FileSystem::Alias Implementation

		// Mount
		//
		// Mounts/binds a foreign node to this alias, obscuring the previous node
		virtual void Mount(const NodePtr&) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }

		// Unmount
		//
		// Unmounts/unbinds a node from this alias, revealing the previously bound node
		virtual void Unmount(void) { throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); }

		// getName
		//
		// Gets the name associated with this alias
		virtual const tchar_t* getName(void) { return m_name; }

		// getNode
		//
		// Accesses the node pointed to by this alias
		virtual FileSystem::NodePtr getNode(void) { return shared_from_this(); }

		//---------------------------------------------------------------------
		// FileSystem::Node Implementation

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const tchar_t* name);

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const tchar_t* name, const tchar_t* target);
	
		// ResolvePath
		//
		// Resolves a path for an alias that is a child of this alias
		virtual FileSystem::AliasPtr ResolvePath(const tchar_t* path, bool follow);

		// getIndex
		//
		// Gets the node index
		virtual uint32_t getIndex(void) { return 12345; /* TODO */ }

		// getType
		//
		// Gets the node type
		virtual NodeType getType(void) { return m_type; }

		//---------------------------------------------------------------------
		// Private Member Functions

		// AppendToPath
		//
		// Appends additional path information to this node's path
		std::vector<tchar_t> AppendToPath(const tchar_t* more);

		//---------------------------------------------------------------------
		// Member Variables

		HANDLE							m_handle;		// Operating system handle
		const tchar_t*					m_name;			// Name portion of the path
		std::vector<tchar_t>			m_path;			// Full path to the host node
		std::shared_ptr<SuperBlock>		m_superblock;	// Reference to the superblock
		const FileSystem::NodeType		m_type;			// Represented node type
	};

	// View
	//
	// Specialization of FileSystem::View for a host file system instance
	class View : public FileSystem::View
	{
	public:

		// Destructor
		//
		virtual ~View()=default;

	private:

		View(const View&)=delete;
		View& operator=(const View&)=delete;

		//---------------------------------------------------------------------
		// FileSystem::View Implementation

		//---------------------------------------------------------------------
		// Member Variables
	};

	// Instance Constructor
	//
	HostFileSystem(const std::shared_ptr<SuperBlock>& superblock, const std::shared_ptr<Node>& root);
	friend class std::_Ref_count_obj<HostFileSystem>;

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// getRoot
	//
	// Accesses the root alias for the file system
	virtual AliasPtr getRoot(void) { return m_root; }

	//-------------------------------------------------------------------------
	// Private Member Functions

	// NodeFromPath
	//
	// Creates a HostFileSystem::Node instance from a path string
	static std::shared_ptr<Node> NodeFromPath(const std::shared_ptr<SuperBlock>& superblock, const tchar_t* path, bool follow);
	static std::shared_ptr<Node> NodeFromPath(const std::shared_ptr<SuperBlock>& superblock, std::vector<tchar_t>&& path, bool follow);

	// NodeTypeFromPath
	//
	// Gets the FileSystem::NodeType for an object from its path
	static FileSystem::NodeType NodeTypeFromPath(const tchar_t* path, DWORD* attributes = nullptr);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<SuperBlock>	m_superblock;	// Contained superblock
	std::shared_ptr<Node>		m_root;			// Mounted root object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_