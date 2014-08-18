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

#include <filesystem>
#include <memory>
#include <type_traits>
#include <concurrent_unordered_map.h>
#include <linux/fs.h>
#include <linux/types.h>
#include "DirectoryEntry.h"
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmFileSystem
//
// todo: words

class VmFileSystem
{
public:

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
	void CreateDirectory(const tchar_t* path, uapi::mode_t mode);

	// Mount
	//
	// Mounts a file system at the specified path
	//void Mount(const tchar_t* path, const FileSystemPtr& mountfs);

	// Unmount
	//
	// Unmounts a file system from the specified path
	//void Unmount(const tchar_t* path);

	//-------------------------------------------------------------------------
	// Propertes
	
	// RootDirectory
	// todo: this probably doesn't need to be exposed at all
	// Accesses the file system root directory entry instance
	//__declspec(property(get=getRootDirectory)) DirectoryEntryPtr RootDirectory;
	//DirectoryEntryPtr getRootDirectory(void) { return m_rootdir; }

private:

	VmFileSystem(const VmFileSystem&)=delete;
	VmFileSystem& operator=(const VmFileSystem&)=delete;

	// Instance Constructor
	//
	VmFileSystem(const FileSystemPtr& rootfs);
	friend std::unique_ptr<VmFileSystem> std::make_unique<VmFileSystem, FileSystemPtr>(FileSystemPtr&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	DirectoryEntryPtr ResolvePath(const tchar_t* path) { return ResolvePath(m_rootdir, path); }
	DirectoryEntryPtr ResolvePath(const DirectoryEntryPtr& base, const tchar_t* path);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// tpath
	//
	// Typedef for a generic text std::tr2::sys::[w]path
	using tpath = std::conditional<sizeof(TCHAR) == sizeof(wchar_t), std::tr2::sys::wpath, std::tr2::sys::path>::type;

	using mounts_collection = Concurrency::concurrent_unordered_map<DirectoryEntryPtr, FileSystemPtr>;

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<FileSystem>		m_rootfs;		// Root file system

	DirectoryEntryPtr m_rootdir;

	mounts_collection m_mounts;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMFILESYSTEM_H_
