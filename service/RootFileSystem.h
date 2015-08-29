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

#ifndef __ROOTFILESYSTEM_H_
#define __ROOTFILESYSTEM_H_
#pragma once

#include <atomic>
#include <memory>
#include <unordered_map>
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem
//
// RootFileSystem implements a virtual single directory node file system in which
// no child nodes can be created
//
// Supported mount options:
//
//	MS_KERNMOUNT
//	MS_NOATIME
//	MS_NODIRATIME
//	MS_RDONLY
//	MS_RELATIME
//	MS_STRICTATIME
//
//	mode=nnn	- Sets the permissions of the directory node
//	uid=nnn		- Sets the owner user id of the directory node
//	gid=nnn		- Sets the owner group id of the directory node
//	
//	(MS_NODEV, MS_NOEXEC and MS_NOSUID are always set)
//
// Supported remount options:
//
//	MS_RDONLY
//
// todo: document weird synchronization of unmount() with m_root and anything else
// that might require clarification

class RootFileSystem
{
public:

	// Instance Constructor
	//
	RootFileSystem(const char_t* source, uint32_t flags);

	// Destructor
	//
	~RootFileSystem()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Mount (static)
	//
	// Creates an instance of the file system
	static std::shared_ptr<FileSystem::Mount> Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength);

private:

	RootFileSystem(const RootFileSystem&)=delete;
	RootFileSystem& operator=(const RootFileSystem&)=delete;

	// Forward Declarations
	//
	class DirectoryHandle;

	// handlemap_t
	//
	// Collection of active DirectoryHandle instances
	using handlemap_t = std::unordered_map<DirectoryHandle*, std::weak_ptr<DirectoryHandle>>;

	// RootFileSystem::DirectoryNode
	//
	class DirectoryNode : public FileSystem::Directory
	{
	public:

		// Instance Constructor
		//
		DirectoryNode(std::shared_ptr<RootFileSystem> fs, uapi::mode_t mode, uapi::uid_t uid, uapi::gid_t gid);

		// Destructor
		//
		~DirectoryNode()=default;

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
		// Looks up the alias associated with a child of this directory
		virtual std::shared_ptr<FileSystem::Alias> Lookup(std::shared_ptr<FileSystem::Mount> mount, const char_t* name) const override;

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

		// Type
		//
		// Gets the type of file system node being implemented
		virtual FileSystem::NodeType getType(void) const override;

	private:

		DirectoryNode(const DirectoryNode&)=delete;
		DirectoryNode& operator=(const DirectoryNode&)=delete;

		//---------------------------------------------------------------------
		// Private Member Functions

		// UpdateAccessTime
		//
		// Updates the access time value of the node
		void UpdateAccessTime(std::shared_ptr<FileSystem::Mount> mount);

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<RootFileSystem>	m_fs;		// Parent file system instance
		datetime								m_ctime;	// Change timestamp
		datetime								m_mtime;	// Modification timestamp
		datetime								m_atime;	// Access timestamp
		uapi::mode_t							m_mode;		// Permission/mode flags
		uapi::uid_t								m_uid;		// Node UID
		uapi::gid_t								m_gid;		// Node GID
		mutable sync::critical_section			m_cs;		// Synchronization object
	};

	// RootFileSystem::DirectoryHandle
	//
	class DirectoryHandle : public FileSystem::Handle
	{
	public:

		// Instance Constructor
		//
		DirectoryHandle(std::shared_ptr<RootFileSystem> fs, FileSystem::HandleAccess access, FileSystem::HandleFlags flags);

		// Destructor
		//
		~DirectoryHandle();

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<Handle> Duplicate(void) const override;

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

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<RootFileSystem>	m_fs;			// Parent file system instance
		const FileSystem::HandleAccess			m_access;		// Handle access mode
		const FileSystem::HandleFlags			m_flags;		// Handle flags
	};

	// RootFileSystem::Mount
	//
	class Mount : public FileSystem::Mount
	{
	public:

		// Instance Constructor
		//
		Mount(std::shared_ptr<RootFileSystem> fs, std::shared_ptr<DirectoryNode> root, uint32_t flags);

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

		const std::shared_ptr<RootFileSystem>	m_fs;		// File system instance
		const uint32_t							m_flags;	// Mounting flags
		std::shared_ptr<DirectoryNode>			m_root;		// The root directory node
	};

	//-------------------------------------------------------------------------
	// Member Variables

	const std::string				m_source;		// Source device name
	std::atomic<uint32_t>			m_flags;		// File system flags
	const uapi::fsid_t				m_fsid;			// File system unique identifier
	handlemap_t						m_handles;		// Active handle instances
	mutable sync::critical_section	m_cs;			// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_