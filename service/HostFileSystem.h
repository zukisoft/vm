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
	virtual ~HostFileSystem()=default;

	// Mount (static)
	//
	// Mounts the file system on the specified device, returns the FileSystem instance
	static std::unique_ptr<FileSystem> Mount(int flags, const char_t* devicename, void* data);

private:

	HostFileSystem(const HostFileSystem&)=delete;
	HostFileSystem& operator=(const HostFileSystem&)=delete;

	// Instance Constructor
	//
	HostFileSystem(const char_t* devicename);
	friend std::unique_ptr<HostFileSystem> std::make_unique<HostFileSystem, const char_t*&>(const char_t*&);

	// Forward Declarations
	//
	class DirectoryEntry;
	class File;
	class Node;

	// Class DirectoryEntry
	//
	// Implementation of FileSystem::DirectoryEntry
	class DirectoryEntry : public FileSystem::DirectoryEntry
	{
	public:

		// Instance Constructors
		//
		DirectoryEntry(const char_t* name, const tchar_t* path);
		// TODO: need one that accepts Node

		// Destructor
		//
		virtual ~DirectoryEntry()=default;

		// Path
		//
		// Exposes the full path to the host object
		__declspec(property(get=getPath)) const tchar_t* Path;
		const tchar_t* getPath(void) const { return m_path.c_str(); }

	private:

		DirectoryEntry(const DirectoryEntry&)=delete;
		DirectoryEntry& operator=(const DirectoryEntry&)=delete;

		// m_name
		//
		// ANSI directory entry name
		std::string m_name;

		// m_path
		//
		// Generic text complete path name
		std::tstring m_path;
	};

	class RootDirectoryEntry : public DirectoryEntry
	{
	public:

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

	private:

		Node(const Node&)=delete;
		Node& operator=(const Node&)=delete;
	};
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOSTFILESYSTEM_H_