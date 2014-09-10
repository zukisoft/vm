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

#include <memory>
#include <type_traits>
#include <concurrent_unordered_map.h>
#include <linux/fcntl.h>
#include <linux/fs.h>
#include <linux/types.h>
#include "LinuxException.h"
#include "FileSystem.h"
#include "PathSplitter.h"

// remove me
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

	// Create (static)
	//
	// Creates a new VmFileSystem instance based on a mounted root file system
	static std::unique_ptr<VmFileSystem> Create(const FileSystemPtr& rootfs);

	// mkdir
	void CreateDirectory(const tchar_t* path);

	// CreateFile
	//
	//
	Handle CreateFile(const tchar_t* path, int flags, uapi::mode_t mode);

	// symlink
	void CreateSymbolicLink(const tchar_t* path, const tchar_t* target);

	// open
	//
	Handle Open(const tchar_t* path, int flags, uapi::mode_t mode);

	// mount
	void Mount(const tchar_t* source, const tchar_t* target, const tchar_t* filesystem, uint32_t flags, void* data);

	// fsync
	//void 

	// umount
	void Unmount(const tchar_t* target, uint32_t flags);

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
	FileSystem::AliasPtr ResolvePath(const tchar_t* absolute);
	FileSystem::AliasPtr ResolvePath(const FileSystem::AliasPtr& base, const tchar_t* relative);

	bool TryResolvePath(const tchar_t* absolute, FileSystem::AliasPtr& result);
	bool TryResolvePath(const FileSystem::AliasPtr& base, const tchar_t* relative, FileSystem::AliasPtr& result);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// mount_map_t
	//
	// Typedef for a concurrent map<> of mounted file systems and the alias they are mounted in
	using mount_map_t = Concurrency::concurrent_unordered_map<FileSystem::AliasPtr, FileSystemPtr>;

	//-------------------------------------------------------------------------
	// Member Variables

	FileSystemPtr				m_rootfs;		// Root file system
	mount_map_t					m_mounts;		// Collection of mounted file systems
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMFILESYSTEM_H_
