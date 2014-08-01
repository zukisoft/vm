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

#include <atomic>
#include <concurrent_queue.h>
#include <mutex>
#include "FileSystem.h"
#include "LinuxException.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)		// inline specifier cannot be used (friend)

//-----------------------------------------------------------------------------
// Class HostFileSystem
//
// Implements a pass-through file system that operates against a source path
// available on the host operating system.
//
// Mount Options:
//
//	TODO
//
// TODO: complete documentation goes here

class HostFileSystem : public FileSystem
{
public:

	// Destructor
	//
	virtual ~HostFileSystem();

	// Mount (static)
	//
	// Mounts the file system on the specified device, returns the FileSystem instance
	static std::unique_ptr<FileSystem> Mount(int flags, const char_t* devicename, void* data);

	// MountPoint (FileSystem)
	//
	// Exposes the file system mount point as a directory entry
	__declspec(property(get=getMountPoint)) std::shared_ptr<FileSystem::DirectoryEntry> MountPoint;
	virtual std::shared_ptr<FileSystem::DirectoryEntry> getMountPoint(void) const { return m_rootalias; }

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// Instance Constructor
	//
	HostFileSystem(const char_t* devicename);
	friend std::unique_ptr<HostFileSystem> std::make_unique<HostFileSystem, const char_t*&>(const char_t*&);

	// AllocateNodeIndex
	//
	// Allocates a node index from the pool
	int32_t AllocateNodeIndex(void);

	// ReleaseNodeIndex
	//
	// Releases a node index from the pool
	void RelaseNodeIndex(int32_t index);

	// Forward Declarations
	//
	class DirectoryEntry;
	class File;
	class Node;

	friend class Node;
	friend class DirectoryEntry;
	friend class File;

	// Class DirectoryEntry
	//
	// Implementation of FileSystem::DirectoryEntry
	class DirectoryEntry : public FileSystem::DirectoryEntry
	{
	public:

		// Instance Constructors
		//
		//DirectoryEntry(const char_t* name);
		DirectoryEntry(const char_t* name, const std::shared_ptr<HostFileSystem::Node>& node);

		// Destructor
		//
		virtual ~DirectoryEntry()=default;

		// Name (FileSystem::DirectoryEntry)
		//
		// Gets the name associated with this directory entry
		__declspec(property(get=getName)) const char_t* Name;
		virtual const char_t* getName(void) const { return m_name.c_str(); }

	private:

		DirectoryEntry(const DirectoryEntry&)=delete;
		DirectoryEntry& operator=(const DirectoryEntry&)=delete;

		// m_name
		//
		// ANSI directory entry name
		std::string m_name;
	};

	class RootDirectoryEntry : public DirectoryEntry
	{
	public:

		RootDirectoryEntry(const char_t* path);

	private:

		RootDirectoryEntry(const RootDirectoryEntry&)=delete;
		RootDirectoryEntry& operator=(const RootDirectoryEntry&)=delete;
	};

	// Class File
	//
	// Implementation of FileSystem::File
	class File : public FileSystem::File
	{
	public:

		// Instance Constructor
		//
		File(const std::shared_ptr<DirectoryEntry>& dentry, const std::shared_ptr<Node>& node);

		// Destructor
		//
		virtual ~File();

		// Read (FileSystem::File)
		//
		// Reads data from the file
		virtual uapi::size_t Read(void* buffer, uapi::size_t count, uapi::loff_t pos);

		// Seek (FileSystem::File)
		//
		// Sets the file pointer
		virtual uapi::loff_t Seek(uapi::loff_t offset, int origin);

		// Sync (FileSystem::File)
		//
		// Flushes the file buffers to the underlying storage
		virtual void Sync(void);

		// Write (FileSystem::File)
		//
		// Synchronously writes data to the file
		virtual uapi::size_t Write(void* buffer, uapi::size_t count, uapi::loff_t pos);

	private:

		File(const File&)=delete;
		File& operator=(const File&)=delete;

		// m_handle
		//
		// Underlying operating system handle
		HANDLE m_handle = INVALID_HANDLE_VALUE;

		// m_lock
		//
		// Recursive mutex to control simultaneous access by multiple threads
		std::recursive_mutex m_lock;

		// m_position
		//
		// Cached file pointer position
		uapi::loff_t m_position = 0;
	};

	// Class Node
	//
	// Implementation of FileSystem::Node
	class Node : public FileSystem::Node
	{
	public:

		// Instance Constructor
		//
		Node(HostFileSystem& fs, HANDLE handle, int32_t index) : m_fs(fs), m_handle(handle), m_index(index) {}

		// Destructor
		//
		virtual ~Node();

		// (FileSystem::Node impl)
		virtual std::shared_ptr<FileSystem::File> OpenFile(const std::shared_ptr<FileSystem::DirectoryEntry>& dentry);

		// Index (FileSystem::Node)
		//
		// Gets the index value for this node
		__declspec(property(get=getIndex)) uint32_t Index;
		virtual uint32_t getIndex(void) const { return static_cast<uint32_t>(m_index); }

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;

		// m_fs
		//
		// Reference to the parent HostFileSystem instance
		HostFileSystem& m_fs;

		// m_handle
		//
		// Handle to the underlying file system object
		HANDLE m_handle = INVALID_HANDLE_VALUE;

		// m_index
		//
		// This node's index (inode) value
		const int32_t m_index;
	};

	// m_nextindex
	//
	// Next sequential node index value
	std::atomic<int32_t> m_nextinode = 0;

	// m_rootalias
	//
	// Maintains a strong reference to the root alias object
	std::shared_ptr<DirectoryEntry> m_rootalias;

	// m_rootnode
	//
	// Maintains a strong reference to the root node object
	std::shared_ptr<Node> m_rootnode;

	// m_spentindexes
	//
	// Queue used to recycle node indexes
	Concurrency::concurrent_queue<int32_t> m_spentinodes;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_