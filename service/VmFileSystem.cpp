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
// VmFileSystem::CreateDirectory
//
// Creates a directory within the file system
//
// Arguments:
//
//	path		- Path to the directory to be created
//	(todo: mode and flags)

void VmFileSystem::CreateDirectory(const uapi::char_t* path)
{
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(splitter.Branch);
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
	
	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(!directory) throw LinuxException(LINUX_ENOTDIR);

	directory->CreateDirectory(branch, splitter.Leaf);
}

VmFileSystem::Handle VmFileSystem::CreateFile(const uapi::char_t* path, int flags, uapi::mode_t mode)
{
	(mode);		// TODO

	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// O_PATH and O_DIRECTORY cannot be used when creating a regular file object
	if((flags & LINUX_O_PATH) || (flags & LINUX_O_DIRECTORY)) throw LinuxException(LINUX_EINVAL);

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(splitter.Branch);
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(!directory) throw LinuxException(LINUX_ENOTDIR);

	return directory->CreateFile(branch, splitter.Leaf, flags);
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

void VmFileSystem::CreateSymbolicLink(const uapi::char_t* path, const uapi::char_t* target)
{
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);
	if((target == nullptr) || (*target == 0)) throw LinuxException(LINUX_ENOENT);

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(splitter.Branch);
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(!directory) throw LinuxException(LINUX_ENOTDIR);

	directory->CreateSymbolicLink(branch, splitter.Leaf, target);
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
// VmFileSystem::Open
//
// Opens or creates a file system object
//
// Arguments:
//
//	base		- Base alias from which to resolve the path
//	path		- Path to the object to be opened/created
//	flags		- Flags indicating how the object should be opened
//	mode		- Mode bitmask to use if a new object is created

VmFileSystem::Handle VmFileSystem::Open(const FileSystem::AliasPtr& base, const uapi::char_t* path, int flags, uapi::mode_t mode)
{
	if(path == nullptr) throw LinuxException(LINUX_EFAULT);
	if(*path == 0) throw LinuxException(LINUX_ENOENT);

	// O_PATH filter -> Only O_CLOEXEC, O_DIRECTORY and O_NOFOLLOW are evaluated
	if(flags & LINUX_O_PATH) flags &= (LINUX_O_PATH | LINUX_O_CLOEXEC | LINUX_O_DIRECTORY | LINUX_O_NOFOLLOW);

	// O_CREAT | O_EXCL indicates that a file object must be created, call CreateFile() instead
	if((flags & (LINUX_O_CREAT | LINUX_O_EXCL)) == (LINUX_O_CREAT | LINUX_O_EXCL)) return CreateFile(path, flags, mode);

	// O_CREAT indicates that if the object does not exist, a new file will be created
	else if((flags & LINUX_O_CREAT) == LINUX_O_CREAT) {

		PathSplitter splitter(path);				// Path splitter

		// Resolve the branch path to an Alias instance, must resolve to a Directory
		auto branch = ResolvePath(base, splitter.Branch);
		if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

		// Ask the branch node to resolve the leaf, if that succeeds, just open it
		FileSystem::AliasPtr leaf;
		if(TryResolvePath(branch, splitter.Leaf, leaf)) return leaf->Node->Open(leaf, flags);

		// The leaf didn't exist (or some other issue happened, TryResolvePath() doesn't
		// discriminate), cast out the branch as a Directory node and create a file
		auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
		if(!directory) throw LinuxException(LINUX_ENOTDIR);

		return directory->CreateFile(branch, splitter.Leaf, flags);
	}

	// Standard open, will throw exception if the object does not exist
	else {
	
		auto alias = ResolvePath(base, path);
		return alias->Node->Open(alias, flags);
	}
}

VmFileSystem::Handle VmFileSystem::OpenExec(const uapi::char_t* path)
{
	if(path == nullptr) throw LinuxException(LINUX_EFAULT);
	if(*path == 0) throw LinuxException(LINUX_ENOENT);

	// todo: Remove flags from OpenExec?  Nothing can be specified by the user
	auto alias = ResolvePath(path);
	auto node = std::dynamic_pointer_cast<FileSystem::File>(alias->Node);
	if(!node) throw LinuxException(LINUX_ENOENT);		// <-- todo: Exception

	return node->OpenExec(alias, 0);
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
//	base		- Base alias instance to use for resolution
//	relative	- Relative path to resolve

FileSystem::AliasPtr VmFileSystem::ResolvePath(const FileSystem::AliasPtr& base, const uapi::char_t* relative)
{
	// Need to know if absolute or relative, if absolute start at root node otherwise base
	// If base is null, start at root also
	//PathSplitter path(relative);


	_ASSERTE(base);
	int symlinks = 0;
	return base->Node->Resolve(m_rootfs->Root, base, relative, 0, &symlinks);		// <--- todo flags
}

bool VmFileSystem::TryResolvePath(const uapi::char_t* absolute, FileSystem::AliasPtr& result)
{
	// TODO: this could be done more efficiently
	// a performance problem
	try { result = ResolvePath(absolute); }
	catch(...) { return false; }

	return true;
}

bool VmFileSystem::TryResolvePath(const FileSystem::AliasPtr& base, const uapi::char_t* relative, FileSystem::AliasPtr& result)
{
	// TODO: this could be done more efficiently
	try { result = ResolvePath(base, relative); }
	catch(...) { return false; }

	return true;
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
