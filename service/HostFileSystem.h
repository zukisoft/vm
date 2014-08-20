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
// todo: prevent cross-volume/cross-mount opens; will need to check the path
// of the opened object against the mounted root path to ensure that people
// can't create a symlink to C:\Windows or something.  If there is already
// a hard link on the host, there isn't much that can be done about that

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
	static FileSystemPtr Mount(const tchar_t* device);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

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

	// Node
	//
	// Specialization of FileSystem::Node for a host file system instance
	class Node : public FileSystem::Node, public FileSystem::Alias, public std::enable_shared_from_this<Node>
	{
	public:

		// Constructor / Destructor
		//
		Node(std::vector<tchar_t>&& path, FileSystem::NodeType type, HANDLE handle);
		virtual ~Node();

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		//---------------------------------------------------------------------
		// FileSystem::Alias Implementation

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
		virtual FileSystem::AliasPtr ResolvePath(const tchar_t* path);

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
		const FileSystem::NodeType		m_type;			// Represented node type
	};

	// Instance Constructor
	//
	HostFileSystem(const std::shared_ptr<Node>& rootnode);
	friend class std::_Ref_count_obj<HostFileSystem>;

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// getRootNode
	//
	// Accesses the root node for the file system
	virtual NodePtr getRootNode(void) { return m_rootnode; }

	//-------------------------------------------------------------------------
	// Private Member Functions

	// NodeFromPath
	//
	// Creates a HostFileSystem::Node instance from a path string
	static std::shared_ptr<Node> NodeFromPath(const tchar_t* path);
	static std::shared_ptr<Node> NodeFromPath(std::vector<tchar_t>&& path);

	// NodeTypeFromPath
	//
	// Gets the FileSystem::NodeType for an object from its path
	static FileSystem::NodeType NodeTypeFromPath(const tchar_t* path);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<Node>		m_rootnode;		// Mounted root node object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_