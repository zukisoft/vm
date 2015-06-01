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

#include "stdafx.h"
#include "RootFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem Constructor (private)
//
// Arguments:
//
//	source		- Source/device name to use for the file system

RootFileSystem::RootFileSystem(const char_t* source) : m_source(source)
{
	_ASSERTE(source);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount (static)
//
// Creates an instance of the file system
//
// Arguments:
//
//	source		- Source device path (ignored)
//	flags		- Standard mount options bitmask
//	data		- Extended mount options data
//	datalength	- Length of the extended mount options data in bytes

std::shared_ptr<FileSystem::Mount> RootFileSystem::Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength)
{
	if(source == nullptr) throw LinuxException(LINUX_EFAULT);

	// Construct the file system instance, using whatever string was passed as a source
	auto fs = std::make_shared<RootFileSystem>(source);

	// todo
	return nullptr;
}

//-----------------------------------------------------------------------------
// RootFileSystem::CreateCharacterDevice (private)
//
// Creates a new character device node within the file system
//
// Arguments:
//
//	parent		- Parent alias to use when creating the new node
//	name		- Name to assign to the newly created alias
//	mode		- Mode flags to assign to the newly created node
//	device		- Major and minor numbers of the newly created character device

void RootFileSystem::CreateCharacterDevice(const std::shared_ptr<FileSystem::Alias>& parent, const char_t* name, uapi::mode_t mode, uapi::dev_t device)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(device);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::CreateDirectory (private)
//
// Creates a new directory node within the file system
//
// Arguments:
//
//	parent		- Parent alias to use when creating the new node
//	name		- Name to assign to the newly created alias
//	mode		- Mode flags to assign to the newly created node

void RootFileSystem::CreateDirectory(const std::shared_ptr<FileSystem::Alias>& parent, const char_t* name, uapi::mode_t mode)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(mode);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::CreateFile (private)
//
// Creates a new regular file node within the file system
//
// Arguments:
//
//	parent		- Parent alias to use when creating the new node
//	name		- Name to assign to the newly created alias
//	flags		- File node access, creation and status flags
//	mode		- Mode flags to assign to the newly created node

std::shared_ptr<FileSystem::Handle> RootFileSystem::CreateFile(const std::shared_ptr<FileSystem::Alias>& parent, const char_t* name, int flags, uapi::mode_t mode)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(mode);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::CreateSymbolicLink (private)
//
// Creates a new symbolic link node within the file system
//
// Arguments:
//
//	parent		- Parent alias to use when creating the new node
//	name		- Name to assign to the newly created alias
//	target		- Path to the symbolic link target alias

void RootFileSystem::CreateSymbolicLink(const std::shared_ptr<FileSystem::Alias>& parent, const char_t* name, const char_t* target)
{
	UNREFERENCED_PARAMETER(parent);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(target);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::getFileSystem
//
// Gets a reference to the underlying node's file system

std::shared_ptr<FileSystem> RootFileSystem::getFileSystem(void)
{
	return shared_from_this();
}

//-----------------------------------------------------------------------------
// RootFileSystem::getRoot
//
// Gets a reference to the file system root node

std::shared_ptr<FileSystem::Node> RootFileSystem::getRoot(void)
{
	return shared_from_this();
}

//-----------------------------------------------------------------------------
// RootFileSystem::getSource
//
// Gets the device/name used as the source of the file system

const char_t* RootFileSystem::getSource(void)
{
	return m_source.c_str();
}

//-----------------------------------------------------------------------------
// RootFileSystem::Stat
//
// Provides statistical information about the file system
//
// Arguments:
//
//	stats		- statfs structure to receive statistics

void RootFileSystem::Stat(uapi::statfs* stats)
{
	_ASSERTE(stats);
	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	// note that rootfs is technically tmpfs or ramfs, so return stats
	// accordingly; just check a live system to be sure

	// todo
	memset(stats, 0, sizeof(uapi::statfs));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Stat
//
// Provides statistical information about the node
//
// Arguments:
//
//	stats		- stat structure to receive statistics

void RootFileSystem::Stat(uapi::stat* stats)
{
	_ASSERTE(stats);
	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	// todo
	memset(stats, 0, sizeof(uapi::stat));
}
	
//
// ROOTFILESYSTEM::MOUNT
//

////-----------------------------------------------------------------------------
//// RootFileSystem::Mount::Duplicate
////
//// Duplicates this mount instance
////
//// Arguments:
////
////	NONE
//
//std::shared_ptr<FileSystem::Mount> RootFileSystem::Mount::Duplicate(void) const
//{
//	return nullptr;
//}

////-----------------------------------------------------------------------------
//// RootFileSystem::Mount::getFileSystem
////
//// Gets a reference to the underlying file system instance
//
//std::shared_ptr<FileSystem2> RootFileSystem::Mount::getFileSystem(void)
//{
//	return m_fs;
//}

////-----------------------------------------------------------------------------
//// RootFileSystem::Mount::getOptions
////
//// Gets a pointer to the contained MountOptions instance
//
//const MountOptions* RootFileSystem::Mount::getOptions(void) const
//{
//	return m_options.get();
//}
//
////-----------------------------------------------------------------------------
//// RootFileSystem::Mount::Remount
////
//// Remounts this mount point with different flags and arguments
////
//// Arguments:
////
////	flags		- Standard mount options bitmask
////	data		- Extended mount options data
////	datalen		- Length, in bytes, of the extended mount options data
//
//void RootFileSystem::Mount::Remount(uint32_t flags, const void* data, size_t datalen)
//{
//	// Allowed: MS_RDONLY, MS_SYNCHRONOUS, MS_MANDLOCK
//}

//-----------------------------------------------------------------------------

#pragma warning(pop)
