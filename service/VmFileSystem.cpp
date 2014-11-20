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

#include "stdafx.h"
#include "VmFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// VmFileSystem Constructor
//
// Arguments:
//
//	rootfs		- Mounted FileSystem instance to serve as the root

VmFileSystem::VmFileSystem(const FileSystemPtr& rootfs) : m_rootfs(rootfs)
{
	_ASSERTE(rootfs);
}

//-----------------------------------------------------------------------------
// VmFileSystem::AddFileSystem
//
// Adds a file system to the collection of available file systems
//
// Arguments:
//
//	name		- Name of the file system (e.g., "procfs")
//	mountfunc	- File system mount function

void VmFileSystem::AddFileSystem(const char_t* name, FileSystem::mount_func mountfunc)
{
	std::lock_guard<std::mutex> critsec(m_fslock);

	// Attempt to construct and insert a new entry for the file system
	auto result = m_availfs.insert(std::make_pair(name, mountfunc));
	if(!result.second) throw Win32Exception(ERROR_ALREADY_EXISTS);
}

//-----------------------------------------------------------------------------
// VmFileSystem::Create (static)
//
// Creates a new file system using the provided mount as the absolute root
//
// Arguments:
//
//	rootfs		- FileSystem instance to serve as the absolute root

std::unique_ptr<VmFileSystem> VmFileSystem::Create(const FileSystemPtr& rootfs)
{
	_ASSERTE(rootfs);
	if(rootfs == nullptr) throw LinuxException(LINUX_EINVAL);

	// todo: register the mount point?
	return std::make_unique<VmFileSystem>(rootfs);
}

//-----------------------------------------------------------------------------
// VmFileSystem::Mount
//
// Mounts a file system at the specified target alias
//
// Arguments:
//
//	source		- Source device/directory to be mounted
//	target		- Target alias to mount the file system on
//	filesystem	- Short name of the filesystem to mount
//	flags		- Mounting flags
//	data		- File-system specific mounting options/data

void VmFileSystem::Mount(const uapi::char_t* source, const uapi::char_t* target, const uapi::char_t* filesystem, uint32_t flags, void* data)
{
	std::lock_guard<std::mutex> critsec(m_fslock);

	// Attempt to locate the filesystem by name in the collection
	auto result = m_availfs.find(filesystem);
	if(result == m_availfs.end()) throw LinuxException(LINUX_ENODEV);

	// Create the file system by passing the arguments into it's mount function
	auto mounted = result->second(source, flags, data);

	// Resolve the target alias and check that it's referencing a directory object
	FileSystem::AliasPtr alias = ResolvePath(target);
	if(alias->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// Overmount the target alias with the new file system's root node
	alias->Mount(mounted->Root->Node);

	// File system was successfully mounted, insert it into the member collection.
	// This will keep both the alias and the file system alive
	m_mounts.insert(std::make_pair(alias, mounted));
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from an absolute file system path
//
// Arguments:
//
//	absolute	- Absolute path to the alias to resolve

FileSystem::AliasPtr VmFileSystem::ResolvePath(const uapi::char_t* absolute)
{
	if(absolute == nullptr) throw LinuxException(LINUX_ENOENT);

	// Remove leading slashes from the provided path and start at the root node
	while((*absolute) && (*absolute == _T('/'))) absolute++;
	return ResolvePath(m_rootfs->Root, absolute);
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from a path relative to an existing alias
//
// Arguments:
//
//	root		- Alias representing the root directory
//	base		- Alias representing the current directory
//	path		- Path to resolve

FileSystem::AliasPtr VmFileSystem::ResolvePath(const FileSystem::AliasPtr& base, const uapi::char_t* path)
{
	_ASSERTE(base);

	// The path is always considered relative here, strip leading slash characters
	while((path) && (*path == '/')) path++;

	int symlinks = 0;
	return base->Node->Resolve(m_rootfs->Root, base, path, 0, &symlinks);		// <--- todo flags
}

//-----------------------------------------------------------------------------
// VmFileSystem::Unmount
//
// Unmounts a mounted file system from it's target alias
//
// Arguments:
//
//	target		- Target alias where file system is mounted
//	flags		- Flags to control unmounting operation

void VmFileSystem::Unmount(const uapi::char_t* target, uint32_t flags)
{
	UNREFERENCED_PARAMETER(target);
	UNREFERENCED_PARAMETER(flags);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
