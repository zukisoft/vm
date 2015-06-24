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
#include <concrt.h>
#include <linux/magic.h>
#include <linux/time.h>
#include "Capabilities.h"
#include "FileSystem.h"
#include "LinuxException.h"
#include "MountOptions.h"
#include "Namespace.h"
#include "PathIterator.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem
//
// RootFileSystem implements a virtual single directory node file system in which
// no additional objects can be created
//
// Supported mount options:
//
//	TODO - LIST FLAGS THAT WILL BE SUPPORTED DURING MOUNT
//	So far: MS_RDONLY, MS_KERNMOUNT
//
//	mode=nnn	- Sets the permissions of the directory node
//	uid=nnn		- Sets the owner user id of the directory node
//	gid=nnn		- Sets the owner group id of the directory node
//	
// Supported remount options:
//
//	MS_RDONLY
//
// TODO: NEED TO TRACK OPEN HANDLES; REGARDLESS OF OWNER IN ORDER
// TO KNOW IF CAN BE UNMOUNTED / REMOUNTED (This is getting complicated)

class RootFileSystem : public FileSystem
{
public:

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

	// RootFileSystem::Directory
	//
	class Directory : public FileSystem::Directory, public std::enable_shared_from_this<Directory>
	{
	public:

		// Destructor
		//
		~Directory()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Create (static)
		//
		// Creates a Directory instance
		static std::shared_ptr<Directory> Create(uapi::mode_t mode, uapi::pid_t uid, uapi::pid_t gid);

		//---------------------------------------------------------------------
		// FileSystem::Directory Implementation

		// CreateCharacterDevice
		//
		// Creates a new character device node as a child of this node
		virtual void CreateCharacterDevice(const std::unique_ptr<FileSystem::Path>& thispath, const char_t* name, uapi::mode_t mode, uapi::dev_t device);

		// CreateDirectory
		//
		// Creates a new directory node as a child of this node
		virtual void CreateDirectory(const std::unique_ptr<FileSystem::Path>& thispath, const char_t* name, uapi::mode_t mode);

		// CreateFile
		//
		// Creates a new regular file node as a child of this node
		virtual std::shared_ptr<Handle> CreateFile(const std::unique_ptr<FileSystem::Path>& thispath, const char_t* name, int flags, uapi::mode_t mode);

		// CreateSymbolicLink
		//
		// Creates a new symbolic link as a child of this node
		virtual void CreateSymbolicLink(const std::unique_ptr<FileSystem::Path>& thispath, const char_t* name, const char_t* target);

		// DemandPermission
		//
		// Demands read/write/execute permissions for the node (MAY_READ, MAY_WRITE, MAY_EXECUTE)
		virtual void DemandPermission(FileSystem::Permission permission);

		// Lookup
		//
		// Resolves a file system path using this node as the starting point
		virtual std::unique_ptr<FileSystem::Path> Lookup(const std::shared_ptr<Namespace>& ns, const std::unique_ptr<Path>& root, 
			const std::unique_ptr<Path>& current, const char_t* path, int flags, int* reparses);
		
		// Open
		//
		// Creates a Handle instance against this node
		virtual std::shared_ptr<Handle> Open(const std::unique_ptr<Path>& thispath, int flags);

		// Stat
		//
		// Provides statistical information about the node
		virtual void Stat(uapi::stat* stats);

		// getType
		//
		// Gets the type of node represented by this object
		virtual FileSystem::NodeType getType(void);

	private:

		Directory(const Directory&)=delete;
		Directory& operator=(const Directory&)=delete;

		// Instance Constructor
		//
		Directory(uapi::mode_t mode, uapi::pid_t uid, uapi::pid_t gid);
		friend class std::_Ref_count_obj<Directory>;

		
		// OpenPath
		//
		// Creates a PathHandle instance against this node
		std::shared_ptr<FileSystem::Handle> OpenPath(const std::unique_ptr<Path>& thispath, int flags);

		//---------------------------------------------------------------------
		// Member Variables

		uapi::stat						m_stats;		// Node statistics
		Concurrency::critical_section	m_statslock;	// Synchronization object
	};

	// RootFileSystem::DirectoryHandle
	//
	class DirectoryHandle : public FileSystem::Handle
	{
	public:

		// Destructor
		//
		~DirectoryHandle()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Create (static)
		//
		// Creates a new DirectoryHandle instance
		static std::shared_ptr<DirectoryHandle> Create(const std::unique_ptr<FileSystem::Path>& path, const std::shared_ptr<Directory>& node);

		//---------------------------------------------------------------------
		// FileSystem::Handle Implementation

		// Duplicate
		//
		// Creates a duplicate Handle instance
		virtual std::shared_ptr<Handle> Duplicate(int flags);

		// Read
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t Read(void* buffer, uapi::size_t count);

		// ReadAt
		//
		// Synchronously reads data from the underlying node into a buffer
		virtual uapi::size_t ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count);

		// Seek
		//
		// Changes the file position
		virtual uapi::loff_t Seek(uapi::loff_t offset, int whence);

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

		// WriteAt
		//
		// Synchronously writes data from a buffer to the underlying node
		virtual uapi::size_t WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count);

	private:

		DirectoryHandle(const DirectoryHandle&)=delete;
		DirectoryHandle& operator=(const DirectoryHandle&)=delete;

		// Instance Constructor
		//
		DirectoryHandle(const std::unique_ptr<FileSystem::Path>& path, const std::shared_ptr<RootFileSystem::Directory>& node);
		friend class std::_Ref_count_obj<DirectoryHandle>; 

		//---------------------------------------------------------------------
		// Member Variables

		const std::unique_ptr<FileSystem::Path>	m_path;		// Referenced path
		const std::shared_ptr<Directory>		m_node;		// Referenced node
	};

	// RootFileSystem::Mount
	//
	class Mount : public FileSystem::Mount
	{
	public:

		// Destructor
		//
		~Mount()=default;

		//---------------------------------------------------------------------
		// Member Functions

		// Create (static)
		//
		// Creates a Mount instance
		static std::shared_ptr<Mount> Create(const std::shared_ptr<RootFileSystem>& fs, uint32_t flags);

		//---------------------------------------------------------------------
		// FileSystem::Mount Implementation

		// Duplicate
		//
		// Duplicates this mount instance
		virtual std::shared_ptr<FileSystem::Mount> Duplicate(void);

		// Remount
		//
		// Remounts this mount point with different flags and arguments
		virtual void Remount(uint32_t flags, const void* data, size_t datalen);

		// Stat
		//
		// Provides statistical information about the mounted file system
		virtual void Stat(uapi::statfs* stats);

		// getFlags
		//
		// Gets the flags set on this mount, includes file system flags
		virtual uint32_t getFlags(void);

		// getRoot
		//
		// Gets a reference to the root node of the mount point
		virtual std::shared_ptr<FileSystem::Node> getRoot(void);

		// getSource
		//
		// Gets the device/name used as the source of the mount point
		virtual const char_t* getSource(void);

	private:

		Mount(const Mount&)=delete;
		Mount& operator=(const Mount&)=delete;

		// Instance Constructor
		//
		Mount(const std::shared_ptr<RootFileSystem>& fs, uint32_t flags);
		friend class std::_Ref_count_obj<Mount>;

		//---------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<RootFileSystem>	m_fs;		// File system instance
		std::atomic<uint32_t>					m_flags;	// Mounting flags
	};

	// Instance Constructor
	//
	RootFileSystem(const char_t* source, uint32_t flags, const std::shared_ptr<Directory>& root);
	friend class std::_Ref_count_obj<RootFileSystem>;

	//-------------------------------------------------------------------------
	// Member Variables

	const std::string					m_source;		// Source device name
	const std::shared_ptr<Directory>	m_root;			// Root directory object
	uapi::statfs						m_stats;		// File system statistics
	Concurrency::critical_section		m_statslock;	// Synchronization object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __ROOTFILESYSTEM_H_