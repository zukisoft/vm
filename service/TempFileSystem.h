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
#include <concrt.h>
#include <concurrent_unordered_map.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include "LinuxException.h"
#include "FilePermission.h"
#include "FileSystem.h"
#include "MountOptions.h"
#include "PathIterator.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TempFileSystem
//
// FILE SIZE: Limited to size_t (4GB on x86 builds, more than sufficient 
// considering it's not possible to address that much memory)

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

	//-------------------------------------------------------------------------
	// FileSystem Implementation

	// getRoot
	//
	// Accesses the root alias for the file system
	virtual FileSystem::AliasPtr getRoot(void) { return m_root; }

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
	class Alias : public FileSystem::Alias
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
		static std::shared_ptr<Alias> Construct(const tchar_t* name, const FileSystem::NodePtr& node);
		static std::shared_ptr<Alias> Construct(const tchar_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node);

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

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		// Instance Constructor
		//
		Alias(const tchar_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node);
		friend class std::_Ref_count_obj<Alias>;

		//---------------------------------------------------------------------
		// Member Variables
		
		std::mutex							m_lock;		// Synchronization object
		std::tstring						m_name;		// Alias name
		std::stack<FileSystem::NodePtr>		m_mounted;	// Stack of mounted nodes
		std::weak_ptr<FileSystem::Alias>	m_parent;	// Parent alias instance
	};

	// TempFileSystem::NodeBase
	//
	// Provides the base (FileSystem::Node) implementation for all node classes
	class __declspec(novtable) NodeBase
	{
	public:

		// Destructor
		//
		virtual ~NodeBase()=default;

		// getIndex
		//
		// todo
		uint64_t getIndex(void) { return m_index; }

		// getType
		//
		// todo
		NodeType getType(void) { return m_type; }

	protected:

		// Instance Constructor
		//
		NodeBase(const std::shared_ptr<MountPoint>& mountpoint, FileSystem::NodeType type);

		//---------------------------------------------------------------------
		// Protected Member Variables

		const uint64_t					m_index;		// Node index number
		std::shared_ptr<MountPoint>		m_mountpoint;	// Contained mountpoint
		FilePermission					m_permission;	// Node permission
		const FileSystem::NodeType		m_type;			// Node type flag

	private:

		NodeBase(const NodeBase&)=delete;
		NodeBase& operator=(const NodeBase&)=delete;
	};

	// DirectoryNode
	//
	// Specializes NodeBase for a Directory file system object
	class DirectoryNode : public NodeBase, public FileSystem::Directory
	{
	public:

		virtual ~DirectoryNode()=default;

		// Construct
		//
		// Constructs a new DirectoryNode instance
		static std::shared_ptr<DirectoryNode> Construct(const std::shared_ptr<MountPoint>& mountpoint);

		// FileSystem::Node Implementation
		//
		virtual FileSystem::HandlePtr	Open(int flags);
		virtual FileSystem::AliasPtr	Resolve(const FileSystem::AliasPtr& current, const tchar_t* path, FileSystem::ResolveState& state);
		virtual uint64_t				getIndex(void) { return NodeBase::getIndex(); }
		virtual FileSystem::NodeType	getType(void) { return NodeBase::getType(); }

		// FileSystem::Directory Implementation
		//
		virtual void					CreateDirectory(const FileSystem::AliasPtr& parent, const tchar_t* name);
		virtual FileSystem::HandlePtr	CreateFile(const FileSystem::AliasPtr& parent, const tchar_t* name, int flags);
		virtual void					CreateSymbolicLink(const FileSystem::AliasPtr& parent, const tchar_t* name, const tchar_t* target);
		virtual void					RemoveNode(const tchar_t* name);

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;

		// Instance Constructor
		//
		DirectoryNode(const std::shared_ptr<MountPoint>& mountpoint) : NodeBase(mountpoint, FileSystem::NodeType::Directory) {}
		friend class std::_Ref_count_obj<DirectoryNode>;

		//---------------------------------------------------------------------
		// Private Type Declarations

		// AliasSet
		//
		// Collection of child Alias instances
		using AliasMap = Concurrency::concurrent_unordered_map<std::tstring, FileSystem::AliasPtr>;

		//---------------------------------------------------------------------
		// Member Variables

		AliasMap				m_children;			// Collection of child aliases
	};

	// FileNode
	//
	// Specializes NodeBase for a File file system object
	class FileNode : public NodeBase, public FileSystem::Node, public std::enable_shared_from_this<FileNode>
	{
	public:

		virtual ~FileNode()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Construct
		//
		// Constructs a new FileNode instance
		static std::shared_ptr<FileNode> Construct(const std::shared_ptr<MountPoint>& mountpoint);

		// FileSystem::Node Implementation
		//		
		virtual FileSystem::HandlePtr	Open(int flags);
		virtual FileSystem::AliasPtr	Resolve(const FileSystem::AliasPtr& current, const tchar_t* path, FileSystem::ResolveState& state);
		virtual uint64_t				getIndex(void) { return NodeBase::getIndex(); }
		virtual FileSystem::NodeType	getType(void) { return NodeBase::getType(); }

	private:

		FileNode(const FileNode&)=delete;
		FileNode& operator=(const FileNode&)=delete;

		// Instance Constructor
		//
		FileNode(const std::shared_ptr<MountPoint>& mountpoint) : NodeBase(mountpoint, FileSystem::NodeType::File) {}
		friend class std::_Ref_count_obj<FileNode>;

		// FileNode::Handle
		//
		// Specialization of FileSystem::Handle for a FileNode object
		class Handle : public FileSystem::Handle
		{
		public:

			// Consructor / Destructor
			//
			Handle(const std::shared_ptr<FileNode>& node, int flags, const FilePermission& permission);
			~Handle()=default;

			// FileSystem::Handle Implementation
			//
			virtual uapi::size_t	Read(void* buffer, uapi::size_t count);
			virtual uapi::loff_t	Seek(uapi::loff_t offset, int whence);
			virtual void			Sync(void);
			virtual void			SyncData(void);
			virtual uapi::size_t	Write(const void* buffer, uapi::size_t count);

		private:

			Handle(const Handle&)=delete;
			Handle& operator=(const Handle&)=delete;

			// Member Variables
			//
			int							m_flags;		// File control flags
			std::shared_ptr<FileNode>	m_node;			// Node reference
			FilePermission				m_permission;	// File permissions
			std::atomic<size_t>			m_position;		// File position
		};

		// Member Variables
		//
		Concurrency::reader_writer_lock		m_lock;		// Synchronization object
		std::vector<uint8_t>				m_data;		// Underlying file data
	};

	// SymbolicLinkNode
	//
	// Specializes NodeBase for a Symbolic Link file system object
	class SymbolicLinkNode : public NodeBase, public FileSystem::SymbolicLink
	{
	public:

		virtual ~SymbolicLinkNode()=default;

		// Construct
		//
		// Constructs a new SymbolicLinkNode instance
		static std::shared_ptr<SymbolicLinkNode> Construct(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* target);

		// FileSystem::Node Implementation
		//
		virtual FileSystem::HandlePtr	Open(int flags);
		virtual FileSystem::AliasPtr	Resolve(const FileSystem::AliasPtr& current, const tchar_t* path, FileSystem::ResolveState& state);
		virtual uint64_t				getIndex(void) { return NodeBase::getIndex(); }
		virtual FileSystem::NodeType	getType(void) { return NodeBase::getType(); }

		// FileSystem::SymbolicLink Implementation
		//
		virtual uapi::size_t			ReadTarget(tchar_t* buffer, size_t count);

	private:

		SymbolicLinkNode(const SymbolicLinkNode&)=delete;
		SymbolicLinkNode& operator=(const SymbolicLinkNode&)=delete;

		// Instance Constructor
		//
		SymbolicLinkNode(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* target) : 
			NodeBase(mountpoint, FileSystem::NodeType::SymbolicLink), m_target(target) {}
		friend class std::_Ref_count_obj<SymbolicLinkNode>;

		//---------------------------------------------------------------------
		// Member Variables

		std::tstring			m_target;			// Symbolic link target
	};

	//---------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<MountPoint>		m_mountpoint;	// Contained mountpoint
	std::shared_ptr<Alias>			m_root;			// Root Alias instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TEMPFILESYSTEM_H_