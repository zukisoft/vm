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
// VmFileSystem::CreateDirectory
//
// Creates a directory within the file system
//
// Arguments:
//
//	path		- Path to the directory to be created
//	(todo: mode flags)

void VmFileSystem::CreateDirectory(const tchar_t* path)
{
	_ASSERTE(path);
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// <filesystem> is way too inefficient I think - it will do for testing
	// yes, it makes a lot of unnecessary copies

	tpath_t pathstr(path);

	// Pull out the desired leaf name string and remove it from the branch path
	std::tstring leafstr = pathstr.filename();
	pathstr = pathstr.parent_path();

	FileSystem::AliasPtr branch = ResolvePath(pathstr.relative_path().string().c_str());
	if(branch == nullptr) throw LinuxException(LINUX_ENOENT);

	branch->Node->CreateDirectory(leafstr.c_str());
}

//-----------------------------------------------------------------------------
// VmFileSystem::CreateSymbolicLink
//
// Creates a symbolic link within the file system
//
// Arguments:
//
//	path		- Path to the directory to be created
//	target		- Symbolic link target

void VmFileSystem::CreateSymbolicLink(const tchar_t* path, const tchar_t* target)
{
	_ASSERTE(path);
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	tpath_t pathstr(path);

	// Pull out the desired leaf name string and remove it from the branch path
	std::tstring leafstr = pathstr.filename();
	pathstr = pathstr.relative_path().parent_path();

	// <filesystem> is way too inefficient I think - it will do for testing

	FileSystem::AliasPtr branch = ResolvePath(pathstr.string().c_str());
	if(branch == nullptr) throw LinuxException(LINUX_ENOENT);

	branch->Node->CreateSymbolicLink(leafstr.c_str(), target);
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

#include "HostFileSystem.h"	// todo: remove me
void VmFileSystem::Mount(const tchar_t* source, const tchar_t* target, const tchar_t* filesystem, uint32_t flags, void* data)
{
	(source);
	(filesystem); // <--- ENODEV if filesystem is bad/unknown

	// Resolve the target alias and check that it's referencing a directory object
	FileSystem::AliasPtr alias = ResolvePath(target);
	if(alias->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	/// TESTING
	FileSystemPtr hfs = HostFileSystem::Mount(source, flags, data);

	// Overmount the target alias with the new file system's root node
	alias->Mount(hfs->Root->Node);

	// File system was successfully mounted, insert it into the member collection.
	// This will keep both the alias and the file system alive
	m_mounts.insert(std::make_pair(alias, hfs));
}

//-----------------------------------------------------------------------------
// VmFileSystem::Open
//
// Opens or creates a file system object
//
// Arguments:
//
//	path		- Path to the object to be opened/created
//	flags		- Flags indicating how the object should be opened

VmFileSystem::Handle VmFileSystem::Open(const tchar_t* path, int flags)
{
	_ASSERTE(path);
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// placeholder code
	FileSystem::AliasPtr alias = ResolvePath(path);
	return alias->Node->OpenHandle(alias, flags);
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from an absolute file system path
//
// Arguments:
//
//	absolute	- Absolute path to the alias to resolve
//	follow		- Flag to follow the final path component if a symbolic link

FileSystem::AliasPtr VmFileSystem::ResolvePath(const tchar_t* absolute, bool follow)
{
	if(absolute == nullptr) throw LinuxException(LINUX_ENOENT);

	// Remove leading slashes from the provided path and start at the root node
	while((*absolute) && (*absolute == _T('/'))) absolute++;
	return ResolvePath(m_rootfs->Root, absolute, follow);
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from a path relative to an existing alias
//
// Arguments:
//
//	base		- Base alias instance to use for resolution
//	relative	- Relative path to resolve
//	follow		- Flag to follow the final path component if a symbolic link

FileSystem::AliasPtr VmFileSystem::ResolvePath(const FileSystem::AliasPtr& base, const tchar_t* relative, bool follow)
{
	_ASSERTE(base);
	return ResolvePath(base->Node, relative, follow);
}

//-----------------------------------------------------------------------------
// VmFileSystem::ResolvePath (private)
//
// Resolves an alias from a path relative to an existing node
//
// Arguments:
//
//	base		- Base node instance to use for resolution
//	relative	- Relative path to resolve
//	follow		- Flag to follow the final path component if a symbolic link

FileSystem::AliasPtr VmFileSystem::ResolvePath(const FileSystem::NodePtr& base, const tchar_t* relative, bool follow)
{
	_ASSERTE(base);
	return base->ResolvePath(relative, follow);
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

void VmFileSystem::Unmount(const tchar_t* target, uint32_t flags)
{
	UNREFERENCED_PARAMETER(target);
	UNREFERENCED_PARAMETER(flags);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
