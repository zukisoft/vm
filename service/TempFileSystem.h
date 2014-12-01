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
#include <memory>
#include <mutex>
#include <stack>
#include <concrt.h>
#include <concurrent_unordered_map.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/time.h>
#include "LinuxException.h"
#include "FilePermission.h"
#include "FileSystem.h"
#include "MemoryRegion.h"
#include "MountOptions.h"
#include "PathIterator.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TempFileSystem
//
// FILE SIZE: Limited to size_t (4GB on x86 builds, more than sufficient 
// considering it's not possible to address that much memory)

// TODO: O_LARGEFILE support - probably just ignore it, can't be more than size_t

class TempFileSystem : public FileSystem
{
public:

	// Destructor
	virtual ~TempFileSystem()=default;

	// Mount
	//
	// Mounts the file system
	static FileSystemPtr Mount(const uapi::char_t*, uint32_t flags, const void* data, size_t datalen);

	// FileSystem Implementation
	//
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

	//-------------------------------------------------------------------------
	// TempFileSystem::MountPoint
	//
	// State and metadata about the mounted file system, all file system objects
	// will maintain a shared reference to this object
	class MountPoint
	{
	public:

		MountPoint(uint32_t flags, const void* data, size_t datalen);
		~MountPoint()=default;

		// AllocateIndex
		//
		// Allocates a new Node index; just wraps around if necessary
		uint64_t AllocateIndex(void) { return m_nextindex++; }

		// Options
		//
		// Gets a reference to the contained MountOptions instance
		__declspec(property(get=getOptions)) MountOptions& Options;
		MountOptions& getOptions(void) { return m_options; }

	private:

		MountPoint(const MountPoint&)=delete;
		MountPoint& operator=(const MountPoint&)=delete;

		// Member Variables
		//
		MountOptions				m_options;			// Mounting options
		std::atomic<uint64_t>		m_nextindex;		// Next inode index
	};

	//-------------------------------------------------------------------------
	// TempFileSystem::Alias
	//
	class Alias : public FileSystem::Alias
	{
	public:

		virtual ~Alias()=default;

		// Construct (static)
		//
		// Constructs a new Alias instance
		static std::shared_ptr<Alias> Construct(const uapi::char_t* name, const FileSystem::NodePtr& node);
		static std::shared_ptr<Alias> Construct(const uapi::char_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node);

		// FileSystem::Alias Implementation
		//
		virtual void					Mount(const FileSystem::NodePtr& node);
		virtual void					Unmount(void);
		virtual const uapi::char_t*		getName(void) { return m_name.c_str(); }
		virtual FileSystem::NodePtr		getNode(void);
		virtual FileSystem::AliasPtr	getParent(void);

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		// Instance Constructor
		//
		Alias(const uapi::char_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node);
		friend class std::_Ref_count_obj<Alias>;

		// Member Variables
		//		
		std::mutex							m_lock;			// Synchronization object
		std::string							m_name;			// Alias name
		std::stack<FileSystem::NodePtr>		m_mounted;		// Stack of mounted nodes
		std::weak_ptr<FileSystem::Alias>	m_parent;		// Parent alias instance
	};

	//-------------------------------------------------------------------------
	// TempFileSystem::NodeBase
	//
	// Provides the base FileSystem::Node implementation for all node types
	class __declspec(novtable) NodeBase
	{
	public:

		virtual ~NodeBase()=default;

		void		DemandPermission(uapi::mode_t mode);

		// todo: put comments back
		uint64_t	getIndex(void) { return m_index; }

		// todo: needs locking
		void UpdateLastAccessTime(void) { GetSystemTimeAsFileTime(&m_atime); }
		void UpdateLastWriteTime(void) { GetSystemTimeAsFileTime(&m_mtime); }
		void UpdateLastChangeTime(void) { GetSystemTimeAsFileTime(&m_ctime); m_mtime = m_ctime; }

		virtual void TESTSTAT(uapi::stat* stats)
		{
			if(stats == nullptr) throw LinuxException(LINUX_EFAULT);
			memset(stats, 0, sizeof(uapi::stat));
			
			stats->st_dev = 0;			// TODO
			stats->st_mode = 0;			// TODO
			
			stats->st_nlink = 1;		// TODO - when hard links are implemented
			stats->st_uid = 0;			// TODO
			stats->st_gid = 0;			// TODO
			stats->st_rdev = 0;			// TODO
			
			stats->st_blksize = MemoryRegion::PageSize;	// TODO - remove "MemoryRegion.h" later, this should be in m_mountpoint's metadata
			stats->st_blocks = 1;						// TODO - needs to come from the FileNode

			// lock times here
			uapi::FILETIMEToTimeSpec(m_atime, &stats->st_atime, &stats->st_atime_nsec);
			uapi::FILETIMEToTimeSpec(m_mtime, &stats->st_mtime, &stats->st_mtime_nsec);
			uapi::FILETIMEToTimeSpec(m_ctime, &stats->st_ctime, &stats->st_ctime_nsec);
			// unlock times here

			stats->st_ino = m_index;
		}

	protected:

		// Instance Constructor
		//
		NodeBase(const std::shared_ptr<MountPoint>& mountpoint);

		// Protected Member Variables
		//
		const uint64_t					m_index;		// Node index number
		std::shared_ptr<MountPoint>		m_mountpoint;	// Contained mountpoint
		FilePermission					m_permission;	// Node permission

		// need a mutex object here or something to protect times
		FILETIME						m_atime;		// Last access time
		FILETIME						m_mtime;		// Last write time
		FILETIME						m_ctime;		// Last Change time

	private:

		NodeBase(const NodeBase&)=delete;
		NodeBase& operator=(const NodeBase&)=delete;
	};

	//-------------------------------------------------------------------------
	// TempFileSystem::PathHandle
	//
	// Specializes FileSystem::Handle for an O_PATH-based handle.  Most operations will fail with
	// EBADF, O_PATH handles can only be used with operations that act on the Handle itself and
	// do not actually access the underlying file
	class PathHandle : public FileSystem::Handle
	{
	public:

		// Constructor / Destructor
		//
		PathHandle(const std::shared_ptr<NodeBase>& node, const std::shared_ptr<FileSystem::Alias>& alias, int flags, const FilePermission& permission) 
			: m_node(node), m_alias(alias), m_flags(flags), m_permission(permission) {}
		~PathHandle()=default;

		// FileSystem::Handle Implementation
		//
		virtual FileSystem::HandlePtr	Duplicate(int)						{ throw LinuxException(LINUX_EBADF); }	// <-- TODO: This should be able to work
		virtual uapi::size_t			Read(void*, uapi::size_t)			{ throw LinuxException(LINUX_EBADF); }
		virtual uapi::loff_t			Seek(uapi::loff_t, int)				{ throw LinuxException(LINUX_EBADF); }
		virtual void					Sync(void)							{ throw LinuxException(LINUX_EBADF); }
		virtual void					SyncData(void)						{ throw LinuxException(LINUX_EBADF); }
		virtual uapi::size_t			Write(const void*, uapi::size_t)	{ throw LinuxException(LINUX_EBADF); }

		virtual FileSystem::AliasPtr	getAlias(void) { return m_alias; }
		virtual int						getFlags(void) { return m_flags; }

	private:

		PathHandle(const PathHandle&)=delete;
		PathHandle& operator=(const PathHandle&)=delete;

		// Member Variables
		//		
		int								m_flags;		// File control flags
		std::shared_ptr<FileSystem::Alias> m_alias;		// Alias instance for this handle
		std::shared_ptr<NodeBase>		m_node;			// Node reference
		FilePermission					m_permission;	// Object permissions
	};

	//-------------------------------------------------------------------------
	// TempFileSystem::DirectoryNode
	//
	// Specializes NodeBase for a Directory file system object
	class DirectoryNode : public NodeBase, public FileSystem::Directory, public std::enable_shared_from_this<DirectoryNode>
	{
	public:

		virtual ~DirectoryNode()=default;

		// Construct
		//
		// Constructs a new DirectoryNode instance
		static std::shared_ptr<DirectoryNode> Construct(const std::shared_ptr<MountPoint>& mountpoint);

		// FileSystem::Node Implementation
		//
		virtual void					DemandPermission(uapi::mode_t mode) { NodeBase::DemandPermission(mode); }
		virtual FileSystem::HandlePtr	Open(const AliasPtr& alias, int flags);
		virtual FileSystem::AliasPtr	Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks);
		virtual uint64_t				getIndex(void)	{ return NodeBase::getIndex(); }
		virtual FileSystem::NodeType	getType(void)	{ return FileSystem::NodeType::Directory; }

		// FileSystem::Directory Implementation
		//
		virtual void					CreateCharacterDevice(const AliasPtr& parent, const uapi::char_t* name, uapi::mode_t mode, uapi::dev_t device);
		virtual void					CreateDirectory(const FileSystem::AliasPtr& parent, const uapi::char_t* name, uapi::mode_t mode);
		virtual FileSystem::HandlePtr	CreateFile(const FileSystem::AliasPtr& parent, const uapi::char_t* name, int flags, uapi::mode_t mode);
		virtual void					CreateSymbolicLink(const FileSystem::AliasPtr& parent, const uapi::char_t* name, const uapi::char_t* target);
		virtual void					RemoveNode(const uapi::char_t* name);

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;

		// Instance Constructor
		//
		DirectoryNode(const std::shared_ptr<MountPoint>& mountpoint) : NodeBase(mountpoint) {}
		friend class std::_Ref_count_obj<DirectoryNode>;

		// DirectoryNode::Handle
		//
		// Specialization of FileSystem::Handle for a DirectoryNode object
		class Handle : public FileSystem::Handle
		{
		public:

			// Consructor / Destructor
			//
			Handle(const std::shared_ptr<DirectoryNode>& node, const std::shared_ptr<FileSystem::Alias>& alias, int flags, const FilePermission& permission) 
				: m_node(node), m_alias(alias), m_flags(flags), m_permission(permission) {}
			~Handle()=default;

			// FileSystem::Handle Implementation
			//
			virtual FileSystem::HandlePtr	Duplicate(int flags)				{ return m_node->Open(m_alias, flags); }
			virtual uapi::size_t			Read(void*, uapi::size_t)			{ throw LinuxException(LINUX_EISDIR); }
			virtual uapi::loff_t			Seek(uapi::loff_t, int)				{ throw LinuxException(LINUX_EISDIR); }
			virtual void					Sync(void)							{ /* do nothing */ }
			virtual void					SyncData(void)						{ /* do nothing */ }
			virtual uapi::size_t			Write(const void*, uapi::size_t)	{ throw LinuxException(LINUX_EISDIR); }

			virtual FileSystem::AliasPtr	getAlias(void) { return m_alias; }
			virtual int						getFlags(void) { return m_flags; }

		private:

			Handle(const Handle&)=delete;
			Handle& operator=(const Handle&)=delete;

			// Member Variables
			//
			int								m_flags;		// File control flags
			std::shared_ptr<FileSystem::Alias> m_alias;		// Alias instance for this handle
			std::shared_ptr<DirectoryNode>	m_node;			// Node reference
			FilePermission					m_permission;	// Directory permissions
		};

		//---------------------------------------------------------------------
		// Private Type Declarations

		// AliasSet
		//
		// Collection of child Alias instances
		using AliasMap = Concurrency::concurrent_unordered_map<std::string, FileSystem::AliasPtr>;

		//---------------------------------------------------------------------
		// Member Variables

		AliasMap				m_children;			// Collection of child aliases
	};

	//-------------------------------------------------------------------------
	// TempFileSystem::FileNode
	//
	// Specializes NodeBase for a File file system object
	class FileNode : public NodeBase, public FileSystem::File, public std::enable_shared_from_this<FileNode>
	{
	public:

		virtual ~FileNode()=default;

		// Construct
		//
		// Constructs a new FileNode instance
		static std::shared_ptr<FileNode> Construct(const std::shared_ptr<MountPoint>& mountpoint);

		// FileSystem::Node Implementation
		//
		virtual void					DemandPermission(uapi::mode_t mode) { NodeBase::DemandPermission(mode); }
		virtual FileSystem::HandlePtr	Open(const AliasPtr& alias, int flags);
		virtual FileSystem::AliasPtr	Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks);
		virtual uint64_t				getIndex(void) { return NodeBase::getIndex(); }
		virtual FileSystem::NodeType	getType(void) { return FileSystem::NodeType::File; }

		virtual void TESTSTAT(uapi::stat* stats)
		{
			NodeBase::TESTSTAT(stats);
			stats->st_mode |= LINUX_S_IFREG;
		}

		// FileSystem::File Implementation
		//		
		virtual FileSystem::HandlePtr	OpenExec(const std::shared_ptr<FileSystem::Alias>& alias);

	private:

		FileNode(const FileNode&)=delete;
		FileNode& operator=(const FileNode&)=delete;

		// Instance Constructor
		//
		FileNode(const std::shared_ptr<MountPoint>& mountpoint) : NodeBase(mountpoint) {}
		friend class std::_Ref_count_obj<FileNode>;

		// FileNode::Handle
		//
		// Specialization of FileSystem::Handle for a FileNode object
		class Handle : public FileSystem::Handle
		{
		public:

			// Constructor / Destructor
			//
			Handle(const std::shared_ptr<FileNode>& node, const std::shared_ptr<FileSystem::Alias>& alias, int flags, const FilePermission& permission)
				: m_flags(flags), m_alias(alias), m_node(node), m_position(0), m_permission(permission) {}
			~Handle()=default;

			// FileSystem::Handle Implementation
			//
			virtual FileSystem::HandlePtr	Duplicate(int flags)	{ return m_node->Open(m_alias, flags); }
			virtual uapi::size_t			Read(void* buffer, uapi::size_t count);
			virtual uapi::loff_t			Seek(uapi::loff_t offset, int whence);
			virtual void					Sync(void)				{ /* do nothing */ }
			virtual void					SyncData(void)			{ /* do nothing */ }
			virtual uapi::size_t			Write(const void* buffer, uapi::size_t count);

			virtual FileSystem::AliasPtr	getAlias(void) { return m_alias; }
			virtual int						getFlags(void) { return m_flags; }

		private:

			Handle(const Handle&)=delete;
			Handle& operator=(const Handle&)=delete;

			// Member Variables
			//
			int							m_flags;		// File control flags
			std::shared_ptr<FileSystem::Alias> m_alias;	// Alias instance used with this handle
			std::shared_ptr<FileNode>	m_node;			// Node reference
			FilePermission				m_permission;	// File permissions
			std::atomic<size_t>			m_position;		// File position
		};

		// Member Variables
		//
		Concurrency::reader_writer_lock		m_lock;		// Synchronization object
		std::vector<uint8_t>				m_data;		// Underlying file data
	};

	//-------------------------------------------------------------------------
	// TempFileSystem::SymbolicLinkNode
	//
	// Specializes NodeBase for a Symbolic Link file system object
	class SymbolicLinkNode : public NodeBase, public FileSystem::SymbolicLink, public std::enable_shared_from_this<SymbolicLinkNode>
	{
	public:

		virtual ~SymbolicLinkNode()=default;

		// Construct
		//
		// Constructs a new SymbolicLinkNode instance
		static std::shared_ptr<SymbolicLinkNode> Construct(const std::shared_ptr<MountPoint>& mountpoint, const uapi::char_t* target);

		// FileSystem::Node Implementation
		//
		virtual void					DemandPermission(uapi::mode_t mode) { NodeBase::DemandPermission(mode); }
		virtual FileSystem::HandlePtr	Open(const AliasPtr& alias, int flags);
		virtual FileSystem::AliasPtr	Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks);
		virtual uint64_t				getIndex(void) { return NodeBase::getIndex(); }
		virtual FileSystem::NodeType	getType(void) { return FileSystem::NodeType::SymbolicLink; }

		// FileSystem::SymbolicLink Implementation
		//
		virtual uapi::size_t			ReadTarget(uapi::char_t* buffer, size_t count);

	private:

		SymbolicLinkNode(const SymbolicLinkNode&)=delete;
		SymbolicLinkNode& operator=(const SymbolicLinkNode&)=delete;

		// Instance Constructor
		//
		SymbolicLinkNode(const std::shared_ptr<MountPoint>& mountpoint, const uapi::char_t* target) : 
			NodeBase(mountpoint), m_target(std::trim(target)) {}
		friend class std::_Ref_count_obj<SymbolicLinkNode>;

		// SymbolicLinkNode::Handle
		//
		// Specialization of FileSystem::Handle for a SymbolicLink object; most operations will
		// simply throw EBADF as this must have been opened with (O_PATH | O_NOFOLLOW)
		class Handle : public FileSystem::Handle
		{
		public:

			// Consructor / Destructor
			//
			Handle(const std::shared_ptr<SymbolicLinkNode>& node, const std::shared_ptr<FileSystem::Alias>& alias, int flags) : 
				m_node(node), m_alias(alias), m_flags(flags) {}
			~Handle()=default;

			// FileSystem::Handle Implementation
			//
			virtual FileSystem::HandlePtr	Duplicate(int flags)				{ return m_node->Open(m_alias, flags); }
			virtual uapi::size_t			Read(void*, uapi::size_t)			{ throw LinuxException(LINUX_EBADF); }
			virtual uapi::loff_t			Seek(uapi::loff_t, int)				{ throw LinuxException(LINUX_EBADF); }
			virtual void					Sync(void)							{ throw LinuxException(LINUX_EBADF); }
			virtual void					SyncData(void)						{ throw LinuxException(LINUX_EBADF); }
			virtual uapi::size_t			Write(const void*, uapi::size_t)	{ throw LinuxException(LINUX_EBADF); }

			virtual FileSystem::AliasPtr	getAlias(void) { return m_alias; }
			virtual int						getFlags(void) { return m_flags; }

		private:

			Handle(const Handle&)=delete;
			Handle& operator=(const Handle&)=delete;

			// Member Variables
			//
			int									m_flags;	// File control flags
			std::shared_ptr<FileSystem::Alias>	m_alias;	// Alias instance
			std::shared_ptr<SymbolicLinkNode>	m_node;		// Node reference
		};

		//---------------------------------------------------------------------
		// Member Variables

		std::string			m_target;			// Symbolic link target
	};

	//-------------------------------------------------------------------------
	// TempFileSystem::CharacterDeviceNode
	//
	// Specializes NodeBase for a CharacterDevice file system object
	class CharacterDeviceNode : public NodeBase, public FileSystem::CharacterDevice, public std::enable_shared_from_this<CharacterDeviceNode>
	{
	public:

		virtual ~CharacterDeviceNode()=default;

		// Construct
		//
		// Constructs a new CharacterDeviceNode instance
		static std::shared_ptr<CharacterDeviceNode> Construct(const std::shared_ptr<MountPoint>& mountpoint, uapi::dev_t device);

		// FileSystem::Node Implementation
		//
		virtual void					DemandPermission(uapi::mode_t mode) { NodeBase::DemandPermission(mode); }
		virtual FileSystem::HandlePtr	Open(const AliasPtr& alias, int flags);
		virtual FileSystem::AliasPtr	Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks);
		virtual uint64_t				getIndex(void) { return NodeBase::getIndex(); }
		virtual FileSystem::NodeType	getType(void) { return FileSystem::NodeType::CharacterDevice; }

	private:

		CharacterDeviceNode(const CharacterDeviceNode&)=delete;
		CharacterDeviceNode& operator=(const CharacterDeviceNode&)=delete;

		// Instance Constructor
		//
		CharacterDeviceNode(const std::shared_ptr<MountPoint>& mountpoint, uapi::dev_t device) : NodeBase(mountpoint), m_device(device) {}
		friend class std::_Ref_count_obj<CharacterDeviceNode>;

		// CharacterDeviceNode::Handle
		//
		// Specialization of FileSystem::Handle for a CharacterDeviceNode object
		class Handle : public FileSystem::Handle
		{
		public:

			// Constructor / Destructor
			//
			// TODO: NEEDS DEVICE ID PASSED IN
			Handle(const std::shared_ptr<CharacterDeviceNode>& node, const std::shared_ptr<FileSystem::Alias>& alias, int flags, const FilePermission& permission)
				: m_flags(flags), m_alias(alias), m_node(node), m_position(0), m_permission(permission) {}
			~Handle()=default;

			// FileSystem::Handle Implementation
			//
			virtual FileSystem::HandlePtr	Duplicate(int flags)	{ return m_node->Open(m_alias, flags); }
			virtual uapi::size_t			Read(void* buffer, uapi::size_t count);
			virtual uapi::loff_t			Seek(uapi::loff_t offset, int whence);
			virtual void					Sync(void)				{ /* do nothing */ }
			virtual void					SyncData(void)			{ /* do nothing */ }
			virtual uapi::size_t			Write(const void* buffer, uapi::size_t count);

			virtual FileSystem::AliasPtr	getAlias(void) { return m_alias; }
			virtual int						getFlags(void) { return m_flags; }

		private:

			Handle(const Handle&)=delete;
			Handle& operator=(const Handle&)=delete;

			// Member Variables
			//
			int										m_flags;		// File control flags
			std::shared_ptr<FileSystem::Alias>		m_alias;		// Alias instance used with this handle
			std::shared_ptr<CharacterDeviceNode>	m_node;			// Node reference
			FilePermission							m_permission;	// File permissions
			std::atomic<size_t>						m_position;		// File position
		};

		// Member Variables
		//
		const uapi::dev_t				m_device;		// Referenced device id
	};

	//---------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<MountPoint>		m_mountpoint;	// Contained mountpoint
	std::shared_ptr<Alias>			m_root;			// Root Alias instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TEMPFILESYSTEM_H_