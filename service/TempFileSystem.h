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

#ifndef __TEMPFILESYSTEM_H_
#define __TEMPFILESYSTEM_H_
#pragma once

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <stack>
#include <linux/types.h>
#include <linux/fcntl.h>
#include "LinuxException.h"
#include "FileSystem.h"
#include "MountOptions.h"
#include "PathIterator.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TempFileSystem
//
// 

class TempFileSystem : public FileSystem
{
public:

	// Destructor
	virtual ~TempFileSystem()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Mount
	//
	// Mounts the file system
	static FileSystemPtr Mount(const tchar_t*, uint32_t flags, void* data);

private:

	TempFileSystem(const TempFileSystem&)=delete;
	TempFileSystem& operator=(const TempFileSystem&)=delete;

	// Forward Declarations
	//
	class Alias;
	class Handle;
	class MountPoint;
	class Node;

	// Instance Constructor
	//
	TempFileSystem(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Alias>& alias);
	friend class std::_Ref_count_obj<TempFileSystem>;

	// TempFileSystem::MountPoint
	//
	// State and metadata about the mounted file system, all file system
	// objects will maintain a shared reference to this object
	class MountPoint
	{
	public:

		// Constructor / Destructor
		//
		MountPoint(uint32_t flags, const void* data);
		~MountPoint()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// AllocateIndex
		//
		// Allocates a new Node index; just wraps around if necessary
		uint64_t AllocateIndex(void) { return m_nextindex++; }

		//---------------------------------------------------------------------
		// Properties

		// Options
		//
		// Gets a reference to the contained MountOptions instance
		__declspec(property(get=getOptions)) MountOptions& Options;
		MountOptions& getOptions(void) { return m_options; }

	private:

		MountPoint(const MountPoint&)=delete;
		MountPoint& operator=(const MountPoint&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		MountOptions				m_options;			// Mounting options
		std::atomic<uint64_t>		m_nextindex;		// Next inode index
	};

	// TempFileSystem::Alias
	//
	// Specialization of FileSystem::Alias for a temp file system instance
	class Alias : public FileSystem::Alias, public std::enable_shared_from_this<Alias>
	{
	public:

		// Constructor
		//
		virtual ~Alias()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Construct (static)
		//
		// Constructs a new Alias instance
		static std::shared_ptr<Alias> Construct(const tchar_t* name, const std::shared_ptr<TempFileSystem::Node>& node);
		static std::shared_ptr<Alias> Construct(const tchar_t* name, const FileSystem::AliasPtr& parent, const std::shared_ptr<TempFileSystem::Node>& node);

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		// Instance Constructor
		//
		Alias(const tchar_t* name, const FileSystem::AliasPtr& parent, const std::shared_ptr<TempFileSystem::Node>& node);
		friend class std::_Ref_count_obj<Alias>;

		//---------------------------------------------------------------------
		// FileSystem::Alias Implementation

		// Mount
		//
		// Mounts/binds a foreign node to this alias, obscuring the previous node
		virtual void Mount(const FileSystem::NodePtr& node);

		// Unmount
		//
		// Unmounts/unbinds a node from this alias, revealing the previously bound node
		virtual void Unmount(void);

		// Name
		//
		// Gets the name associated with this alias
		virtual const tchar_t* getName(void) { return m_name.c_str(); }

		// Node
		//
		// Gets the node instance that this alias references
		virtual FileSystem::NodePtr getNode(void);

		// Parent
		//
		// Gets the parent Alias for this alias instance
		virtual FileSystem::AliasPtr getParent(void);

		//---------------------------------------------------------------------
		// Member Variables
		
		std::mutex							m_lock;		// Synchronization object
		std::tstring						m_name;		// Alias name
		std::stack<FileSystem::NodePtr>		m_mounted;	// Stack of mounted nodes
		FileSystem::AliasPtr				m_parent;	// Parent alias instance
	};

	// TempFileSystem::Node
	//
	// Specialization of FileSystem::Node for a temp file system instance 
	class __declspec(novtable) Node : public FileSystem::Node
	{
	public:

		// Destructor
		//
		virtual ~Node()=default;

	protected:

		// Instance Constructor
		//
		Node(const std::shared_ptr<MountPoint>& mountpoint, FileSystem::NodeType type);

		//---------------------------------------------------------------------
		// Protected Member Variables

		const uint64_t					m_index;		// Node index number
		std::shared_ptr<MountPoint>		m_mountpoint;	// Contained mountpoint
		const FileSystem::NodeType		m_type;			// Node type flag

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		//---------------------------------------------------------------------
		// FileSystem::Node Implementation

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const FileSystem::AliasPtr&, const tchar_t*) { throw LinuxException(LINUX_ENOTDIR); }

		// CreateFile
		//
		// Creates a new regular file node as a child of this node
		virtual FileSystem::HandlePtr CreateFile(const tchar_t*, int) { throw LinuxException(LINUX_ENOTDIR); }

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const FileSystem::AliasPtr&, const tchar_t*, const tchar_t*) { throw LinuxException(LINUX_ENOTDIR); }

		// OpenHandle
		//
		// Creates a FileSystem::Handle instance for this node
		virtual FileSystem::HandlePtr OpenHandle(int flags) = 0;

		// ResolvePath
		//
		// Resolves a relative path from this node to an Alias instance
		virtual FileSystem::AliasPtr ResolvePath(const FileSystem::AliasPtr&, const tchar_t*) { throw LinuxException(LINUX_ENOTDIR); }

		// Index
		//
		// Gets the node index
		virtual uint64_t getIndex(void) { return m_index; }

		// Type
		//
		// Gets the node type
		virtual NodeType getType(void) { return m_type; }
	};

	// DirectoryNode
	//
	// Specializes Node for a Directory file system object
	class DirectoryNode : public Node
	{
	public:

		virtual ~DirectoryNode()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Construct
		//
		// Constructs a new DirectoryNode instance
		static std::shared_ptr<DirectoryNode> Construct(const std::shared_ptr<MountPoint>& mountpoint);

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;

		// Instance Constructor
		//
		DirectoryNode(const std::shared_ptr<MountPoint>& mountpoint) : Node(mountpoint, FileSystem::NodeType::Directory) {}
		friend class std::_Ref_count_obj<DirectoryNode>;

		//---------------------------------------------------------------------
		// Node Specialization

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const FileSystem::AliasPtr& parent, const tchar_t* name);

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const FileSystem::AliasPtr& parent, const tchar_t* name, const tchar_t* target);

		// OpenHandle
		//
		// Creates a FileSystem::Handle instance for this node
		virtual FileSystem::HandlePtr OpenHandle(int flags);
		
		// ResolvePath
		//
		// Resolves a relative path from this node to an Alias instance
		virtual FileSystem::AliasPtr ResolvePath(const FileSystem::AliasPtr& current, const tchar_t* path);

		//---------------------------------------------------------------------
		// Member Variables

		// TODO: temporary, this needs to be thread-safe at minimum and would
		// likely be better served with a higher performing construct
		std::recursive_mutex m_lock;
		std::map<std::tstring, FileSystem::AliasPtr> m_children;
	};

	// FileNode
	//
	// Specializes Node for a File file system object
	class FileNode : public Node
	{
	public:

		virtual ~FileNode()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Construct
		//
		// Constructs a new FileNode instance
		static std::shared_ptr<FileNode> Construct(const std::shared_ptr<MountPoint>& mountpoint);

	private:

		FileNode(const FileNode&)=delete;
		FileNode& operator=(const FileNode&)=delete;

		// Instance Constructor
		//
		FileNode(const std::shared_ptr<MountPoint>& mountpoint) : Node(mountpoint, FileSystem::NodeType::File) {}
		friend class std::_Ref_count_obj<FileNode>;

		//---------------------------------------------------------------------
		// Node Specialization
		
		// OpenHandle
		//
		// Creates a FileSystem::Handle instance for this node
		virtual FileSystem::HandlePtr OpenHandle(int flags);

		//---------------------------------------------------------------------
		// Member Variables

		// TODO: This is temporary just to get things working, it needs to
		// be a lot more fancy than just a vector<>
		std::vector<uint8_t> m_data;
	};

	// SymbolicLinkNode
	//
	// Specializes Node for a Symbolic Link file system object
	class SymbolicLinkNode : public Node
	{
	public:

		virtual ~SymbolicLinkNode()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Construct
		//
		// Constructs a new SymbolicLinkNode instance
		static std::shared_ptr<SymbolicLinkNode> Construct(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* target);

	private:

		SymbolicLinkNode(const SymbolicLinkNode&)=delete;
		SymbolicLinkNode& operator=(const SymbolicLinkNode&)=delete;

		// Instance Constructor
		//
		SymbolicLinkNode(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* target) : Node(mountpoint, FileSystem::NodeType::SymbolicLink), m_target(target) {}
		friend class std::_Ref_count_obj<SymbolicLinkNode>;

		//---------------------------------------------------------------------
		// Node Specialization
		
		// OpenHandle
		//
		// Creates a FileSystem::Handle instance for this node
		virtual FileSystem::HandlePtr OpenHandle(int flags);

		// ResolvePath
		//
		// Resolves a relative path from this node to an Alias instance
		virtual FileSystem::AliasPtr ResolvePath(const FileSystem::AliasPtr& current, const tchar_t* path);

		//---------------------------------------------------------------------
		// Member Variables

		std::tstring			m_target;			// Symbolic link target
	};

	// Handle
	//
	// Specialization of FileSystem::Handle for temp file system instance
	class Handle : public FileSystem::Handle
	{
	public:

		// Constructor / Destructor
		//
		Handle()=default;
		virtual ~Handle()=default;

	private:

		Handle(const Handle&)=delete;
		Handle& operator=(const Handle&)=delete;

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count);

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void);

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void);

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count);

		//---------------------------------------------------------------------
		// Private Member Functions

		//---------------------------------------------------------------------
		// Member Variables
	};

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// getRoot
	//
	// Accesses the root alias for the file system
	virtual FileSystem::AliasPtr getRoot(void) { return m_root; }

	//---------------------------------------------------------------------
	// Private Member Functions

	//---------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<MountPoint>		m_mountpoint;	// Contained mountpoint
	std::shared_ptr<Alias>			m_root;			// Root Alias instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TEMPFILESYSTEM_H_