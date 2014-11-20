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

#ifndef __VMFILESYSTEM_H_
#define __VMFILESYSTEM_H_
#pragma once

#include <map>
#include <memory>
#include <mutex>
#include <type_traits>
#include <concurrent_unordered_map.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/types.h>
#include "LinuxException.h"
#include "FileSystem.h"
#include "PathSplitter.h"
#include "Win32Exception.h"

// todo: remove me?
#include <PathCch.h>

#pragma warning(push, 4)
#pragma warning(disable:4396)		// inline specifier cannot be used when a friend ...

//-----------------------------------------------------------------------------
// VmFileSystem
//
// todo: words

class VmFileSystem
{
public:

	// Handle
	//
	// Simplification of the FileSystem::HandlePtr type, this is the name that
	// should be used by external callers
	using Handle = FileSystem::HandlePtr;

	// Destructor
	//
	~VmFileSystem()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// AddFileSystem
	//
	// Adds a file system to the collection of available file systems
	void AddFileSystem(const char_t* name, FileSystem::mount_func mount_func);

	// Create (static)
	//
	// Creates a new VmFileSystem instance based on a mounted root file system
	static std::unique_ptr<VmFileSystem> Create(const FileSystemPtr& rootfs);

	// mount
	void Mount(const uapi::char_t* source, const uapi::char_t* target, const uapi::char_t* filesystem, uint32_t flags, void* data);

	// umount
	void Unmount(const uapi::char_t* target, uint32_t flags);

	__declspec(property(get=getRoot)) FileSystem::AliasPtr Root;
	FileSystem::AliasPtr getRoot(void) { return m_rootfs->Root; }

private:

	VmFileSystem(const VmFileSystem&)=delete;
	VmFileSystem& operator=(const VmFileSystem&)=delete;

	// Instance Constructor
	//
	VmFileSystem(const FileSystemPtr& rootfs);
	friend std::unique_ptr<VmFileSystem> std::make_unique<VmFileSystem, const FileSystemPtr&>(const FileSystemPtr&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// ResolvePath
	//
	// Resolves an alias instance based on a path
	FileSystem::AliasPtr ResolvePath(const uapi::char_t* absolute);
	FileSystem::AliasPtr ResolvePath(const FileSystem::AliasPtr& base, const uapi::char_t* relative);
	
	//-------------------------------------------------------------------------
	// Private Type Declarations

	// fs_map_t
	//
	// Typedef for a map<> of available file systems
	using fs_map_t = std::map<std::string, FileSystem::mount_func>;

	// mount_map_t
	//
	// Typedef for a concurrent map<> of mounted file systems and the alias they are mounted in
	using mount_map_t = Concurrency::concurrent_unordered_map<FileSystem::AliasPtr, FileSystemPtr>;

	//-------------------------------------------------------------------------
	// Member Variables

	std::mutex					m_fslock;		// File system critical section
	fs_map_t					m_availfs;		// Available file systems

	FileSystemPtr				m_rootfs;		// Root file system
	mount_map_t					m_mounts;		// Collection of mounted file systems
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMFILESYSTEM_H_
