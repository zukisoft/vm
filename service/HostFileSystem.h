//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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

#include <atomic>
#include <memory>
#include <unordered_map>
#include "FileSystem.h"
#include "path.h"

#pragma warning(push, 4)

//----------------------------------------------------------------------------
// HostFileSystem
//
// HostFileSystem implements a pass-through file system that operates against
// a directory accessible to the host operating system
//
// Supported mount options:
//
//	MS_DIRSYNC
//	MS_KERNMOUNT
//	MS_NODEV		(Always set)
//	MS_NOEXEC
//	MS_NOSUID		(Always set)
//	MS_RDONLY
//	MS_SYNCHRONOUS
//
//	[no]sandbox		- Controls sandboxing of the virtual file system (see below)
//	
// Supported remount options:
//
//	MS_RDONLY
//	MS_SYNCHRONOUS
//
// Notes:
//
//	- Sandboxing of the virtual file system is enabled by default, but can be disabled by mounting
//	with a 'nosandbox' option.  This option verifies that any node accessed is *physically* a descendant
//	of the specified mount point.  Consider the ramifications of a symbolic link or junction point
//	that redirects into a place that the user didn't expect it to, like C:\Windows\System32.
//
//	- O_DIRECT: Windows provides the ability to bypass caching like this (FILE_FLAG_NO_BUFFERING), 
//	but all I/O is done in the address space of the RPC server not the address space of the client 
//	application.  Any memory alignment requirements would apply to the RPC server and become 
//	meaningless once the input/output data is marshaled back to the client application.  This flag
//	will be ignored.
//
//	- O_LARGEFILE: Windows doesn't really care about this, and the file system implementation
//	should have no idea if it's being accessed through the 32 or 64 bit interface(s).  This flag
//	will be ignored.
//
//	- O_NOATIME: There is no method on Windows to specify this on an individual file handle, it
//	must be set by an administrator and applies to the entire system.  This flag will be ignored.
//
//	- O_NOFOLLOW: Windows restricts creation of symbolic links to administrators, so they are
//	effectively just ignored by this file system.  When a symbolic link is encountered it will
//	always be followed to the node to which it points on the host file system.  This flag will
//	be ignored.
//
// TODO: O_ASYNC
// TODO: O_NONBLOCK, O_NOCTTY comments - likely to just be ignored flags
// TODO: O_TMPFILE
//
// TODO: The alias objects have to be tracked/cached as well, consider how MountNamespace
// is supposed to work.  It maps an alias instance to a mount, so if a new alias is created
// for a file system object, it won't match

class HostFileSystem
{
public:

	// Instance Constructor
	//
	HostFileSystem(const char_t* source, uint32_t flags);

	// Destructor
	//
	~HostFileSystem()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Mount (static)
	//
	// Creates an instance of the file system
	static std::shared_ptr<FileSystem::Mount> Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// Forward Declarations
	//
	class HandleBase;
	class NodeBase;

	// nodemap_t
	//
	// Collection of active NodeBase instances
	using nodemap_t = std::unordered_map<NodeBase*, std::weak_ptr<NodeBase>>;

	// handlemap_t
	//
	// Collection of active HandleBase instances
	using handlemap_t = std::unordered_map<HandleBase*, std::weak_ptr<HandleBase>>;

	// HostFileSystem::Alias
	//
	class Alias : public FileSystem::Alias
	{
	public:

		// Instance Constructors
		//
		Alias(std::shared_ptr<HostFileSystem> fs, const char* name, std::shared_ptr<FileSystem::Node> node);

		// Destructor
		//
		~Alias()=default;

		//---------------------------------------------------------------------
		// FileSystem::Alias Implementation

		// GetName
		//
		// Reads the name assigned to this alias
		virtual uapi::size_t GetName(char_t* buffer, size_t count) const override;

		// Name
		//
		// Gets the name assigned to this alias
		virtual std::string getName(void) const override;

		// Node
		//
		// Gets the node to which this alias refers
		virtual std::shared_ptr<FileSystem::Node> getNode(void) const override;

	private:

		Alias(const Alias&)=delete;
		Alias& operator=(const Alias&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<HostFileSystem>	m_fs;		// Parent file system instance
		const std::shared_ptr<FileSystem::Node>	m_node;		// Node instance
		std::string								m_name;		// Alias name to report
	};

	// HostFileSystem::HandleBase
	//
	class HandleBase
	{
	public:

		// Instance Constructor
		//
		HandleBase(std::shared_ptr<HostFileSystem> fs, HANDLE handle, FileSystem::HandleAccess access, FileSystem::HandleFlags flags);

		// Destructor
		//
		virtual ~HandleBase();

	protected:

		//---------------------------------------------------------------------
		// Protected Member Variables

		const std::shared_ptr<HostFileSystem>	m_fs;		// Parent file system instance
		HANDLE const							m_handle;	// Native operating system handle
		const FileSystem::HandleAccess			m_access;	// Handle access mode
		const FileSystem::HandleFlags			m_flags;	// Handle flags

	private:

		HandleBase(const HandleBase&)=delete;
		HandleBase& operator=(const HandleBase&)=delete;
	};

	// HostFileSystem::NodeBase
	//
	class NodeBase
	{
	public:

		// Instance Constructor
		//
		NodeBase(std::shared_ptr<HostFileSystem> fs, HANDLE handle);

		// Destructor
		//
		virtual ~NodeBase();

		//---------------------------------------------------------------------
		// Properties

		// Handle
		//
		// Gets the underlying file system object handle (query-only)
		// todo: See if this can be removed
		__declspec(property(get=getHandle)) HANDLE Handle;
		HANDLE getHandle(void) const;

		// NormalizedPath
		//
		// Gets a pointer to the normalized path to this node on the host
		__declspec(property(get=getNormalizedPath)) const wchar_t* NormalizedPath;
		const wchar_t* getNormalizedPath(void) const;

	protected:

		//---------------------------------------------------------------------
		// Protected Member Variables

		const std::shared_ptr<HostFileSystem>	m_fs;		// Parent file system instance
		HANDLE const							m_handle;	// Native operating system handle
		windows_path							m_path;		// Normalized path to this node

	private:

		NodeBase(const NodeBase&)=delete;
		NodeBase& operator=(const NodeBase&)=delete;
	};

	// HostFileSystem::DirectoryHandle
	//
	class DirectoryHandle : public HandleBase, public FileSystem::Handle
	{
	public:

		// Instance Constructor
		//
		using HandleBase::HandleBase;

		// Destructor
		//
		virtual ~DirectoryHandle()=default;

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<FileSystem::Handle> Duplicate(void) const override;

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count) override;

		// ReadAt
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count) override;

		// Seek
		//
		// Changes the file position
		virtual uapi::loff_t Seek(uapi::loff_t offset, int whence) override;

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void) const override;

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void) const override;

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count) override;

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count) override;

		// getAccess
		//
		// Gets the handle access mode
		virtual FileSystem::HandleAccess getAccess(void) const override;

		// getFlags
		//
		// Gets the handle flags
		virtual FileSystem::HandleFlags getFlags(void) const override;

	private:

		DirectoryHandle(const DirectoryHandle&)=delete;
		DirectoryHandle& operator=(const DirectoryHandle&)=delete;
	};

	// HostFileSystem::DirectoryNode
	//
	class DirectoryNode : public NodeBase, public FileSystem::Directory
	{
	public:

		// Instance Constructor
		//
		using NodeBase::NodeBase;

		// Destructor
		//
		virtual ~DirectoryNode()=default;

		//---------------------------------------------------------------------
		// FileSystem::Node Implementation

		// Open
		//
		// Creates a Handle instance against this node
		virtual std::shared_ptr<FileSystem::Handle> Open(std::shared_ptr<FileSystem::Mount> mount, FileSystem::HandleAccess access, FileSystem::HandleFlags flags) override;

		// SetOwnership
		//
		// Changes the ownership of this node
		virtual void SetOwnership(uapi::uid_t uid, uapi::gid_t gid) override;

		// SetPermissions
		//
		// Changes the permission flags for this node
		virtual void SetPermissions(uapi::mode_t permissions) override;

		// Stat
		//
		// Provides statistical information about the node
		virtual void Stat(uapi::stat* stats) const override;

		// getType
		//
		// Gets the type of node being represented in the derived object instance
		virtual FileSystem::NodeType getType(void) const override;

		//---------------------------------------------------------------------
		// FileSystem::Directory Implementation

		// CreateDirectory
		//
		// Creates a new directory node as a child of this directory
		virtual std::shared_ptr<FileSystem::Alias> CreateDirectory(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode) override;

		// CreateFile
		//
		// Creates a new regular file node as a child of this directory
		virtual std::shared_ptr<FileSystem::Alias> CreateFile(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode) override;

		// Lookup
		//
		// Looks up the alias associated with a child of this node
		virtual std::shared_ptr<FileSystem::Alias> Lookup(std::shared_ptr<FileSystem::Mount> mount, const char_t* name) const override;

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;
	};
	
	// HostFileSystem::FileHandle
	//
	class FileHandle : public HandleBase, public FileSystem::Handle
	{
	public:

		// Instance Constructor
		//
		using HandleBase::HandleBase;

		// Destructor
		//
		virtual ~FileHandle()=default;

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<FileSystem::Handle> Duplicate(void) const override;

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count) override;

		// ReadAt
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count) override;

		// Seek
		//
		// Changes the file position
		virtual uapi::loff_t Seek(uapi::loff_t offset, int whence) override;

		// Sync
		//
		// Synchronizes all metadata and data associated with the file to storage
		virtual void Sync(void) const override;

		// SyncData
		//
		// Synchronizes all data associated with the file to storage, not metadata
		virtual void SyncData(void) const override;

		// Write
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t Write(const void* buffer, uapi::size_t count) override;

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count) override;

		// getAccess
		//
		// Gets the handle access mode
		virtual FileSystem::HandleAccess getAccess(void) const override;

		// getFlags
		//
		// Gets the handle flags
		virtual FileSystem::HandleFlags getFlags(void) const override;

	private:

		FileHandle(const FileHandle&)=delete;
		FileHandle& operator=(const FileHandle&)=delete;
	};

	// HostFileSystem::FileNode
	//
	class FileNode : public NodeBase, public FileSystem::File
	{
	public:

		// Instance Constructor
		//
		using NodeBase::NodeBase;

		// Destructor
		//
		virtual ~FileNode()=default;

		//---------------------------------------------------------------------
		// FileSystem::Node Implementation

		// Open
		//
		// Creates a Handle instance against this node
		virtual std::shared_ptr<FileSystem::Handle> Open(std::shared_ptr<FileSystem::Mount> mount, FileSystem::HandleAccess access, FileSystem::HandleFlags flags) override;

		// SetOwnership
		//
		// Changes the ownership of this node
		virtual void SetOwnership(uapi::uid_t uid, uapi::gid_t gid) override;

		// SetPermissions
		//
		// Changes the permission flags for this node
		virtual void SetPermissions(uapi::mode_t permissions) override;

		// Stat
		//
		// Provides statistical information about the node
		virtual void Stat(uapi::stat* stats) const override;

		// getType
		//
		// Gets the type of node being represented in the derived object instance
		virtual FileSystem::NodeType getType(void) const override;

		//---------------------------------------------------------------------
		// FileSystem::File Implementation

		// OpenExec
		//
		// Creates an execute-only handle against this node; used only by the virtual machine
		virtual std::shared_ptr<FileSystem::Handle> OpenExec(std::shared_ptr<FileSystem::Mount> mount) const override;

	private:

		FileNode(const FileNode&)=delete;
		FileNode& operator=(const FileNode&)=delete;
	};

	// HostFileSystem::Mount
	//
	class Mount : public FileSystem::Mount
	{
	public:

		// Instance Constructor
		//
		Mount(std::shared_ptr<HostFileSystem> fs, std::shared_ptr<DirectoryNode> root, uint32_t flags);		

		// Destructor
		//
		~Mount()=default;

		//---------------------------------------------------------------------
		// FileSystem::Mount Implementation

		// Duplicate
		//
		// Duplicates this mount instance
		virtual std::shared_ptr<FileSystem::Mount> Duplicate(void) const override;

		// Remount
		//
		// Remounts this mount point with different flags and arguments
		virtual void Remount(uint32_t flags, const void* data, size_t datalen) override;

		// Stat
		//
		// Provides statistical information about the mounted file system
		virtual void Stat(uapi::statfs* stats) const override;

		// Unmount
		//
		// Unmounts the file system
		virtual void Unmount(void) override;

		// getFlags
		//
		// Gets the flags set on this mount, includes file system flags
		virtual uint32_t getFlags(void) const override;

		// getRoot
		//
		// Gets a reference to the root directory of the mount point
		virtual std::shared_ptr<FileSystem::Directory> getRoot(void) const override;

		// getSource
		//
		// Gets the device/name used as the source of the mount point
		virtual std::string getSource(void) const override;

	private:

		Mount(const Mount&)=delete;
		Mount& operator=(const Mount&)=delete;

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<HostFileSystem>	m_fs;		// File system instance
		const uint32_t							m_flags;	// Mounting flags
		std::shared_ptr<DirectoryNode>			m_root;		// The root directory node/alias
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CreateDirectoryNode (static)
	//
	// Creates a new DirectoryNode instance
	static std::shared_ptr<DirectoryNode> CreateDirectoryNode(std::shared_ptr<HostFileSystem> fs, const windows_path& path);

	// CreateFileNode (static)
	//
	// Creates a new FileNode instance
	static std::shared_ptr<FileNode> CreateFileNode(std::shared_ptr<HostFileSystem> fs, DWORD disposition, const windows_path& path);

	//-------------------------------------------------------------------------
	// Member Variables

	const std::string				m_source;		// File system source string
	std::wstring					m_sandbox;		// Normalized base path string
	std::atomic<uint32_t>			m_flags;		// File system specific flags
	const uapi::fsid_t				m_fsid;			// File system unique identifier
	nodemap_t						m_nodes;		// Active node instances
	handlemap_t						m_handles;		// Active handle instances
	mutable sync::critical_section	m_cs;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_