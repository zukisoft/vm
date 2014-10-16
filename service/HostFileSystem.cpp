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
#include "HostFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// MapHostException (local)
//
// Converts a Win32 error code into a representative LinuxException instance
//
// Arguments:
//
//	code		- Win32 error code to be mapped

static LinuxException MapHostException(DWORD code = GetLastError())
{
	int linuxcode = LINUX_EIO;		// Use EIO as the default linux error code

	// Try to map the Win32 error code to something that makes sense
	switch(code) {

		case ERROR_FILE_NOT_FOUND:		linuxcode = LINUX_ENOENT; break;
		case ERROR_PATH_NOT_FOUND:		linuxcode = LINUX_ENOENT; break;
		case ERROR_FILE_EXISTS:			linuxcode = LINUX_EEXIST; break;
		case ERROR_INVALID_PARAMETER:	linuxcode = LINUX_EINVAL; break;
		case ERROR_ALREADY_EXISTS:		linuxcode = LINUX_EEXIST; break;
	}

	// Generate a LinuxException with the mapped code and provide the underlying Win32
	// error as an inner Win32Exception instance
	return LinuxException(linuxcode, Win32Exception(code));
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::Mount (static)
//
// Mounts the file system on a host system directory object
//
// Arguments:
//
//	source		- Host path; must reference a directory object
//	flags		- Standard mounting flags and attributes
//	data		- Additional file-system specific mounting options

FileSystemPtr HostFileSystem::Mount(const uapi::char_t* source, uint32_t flags, void* data)
{
	std::shared_ptr<MountPoint>			mountpoint;			// MountPoint instance

	std::tstring hostpath = std::to_tstring(source);		// Convert source from ANSI/UTF-8

	// Determine the type of node that the path represents; must be a directory for mounting
	DWORD attributes = GetFileAttributes(hostpath.c_str());
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOTDIR);
	if((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) throw LinuxException(LINUX_ENOTDIR);

	// Attempt to open a query-only handle against the file system directory object
	HANDLE handle = CreateFile(hostpath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 
		FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException();

	// Attempt to create the MountPoint instance for this file system against the opened handle
	try { mountpoint = std::make_shared<MountPoint>(handle, flags, data); }
	catch(...) { CloseHandle(handle); throw; }

	// Construct the HostFileSystem instance , providing an alias attached to the mountpoint node
	return std::make_shared<HostFileSystem>(mountpoint, Alias::Construct("", nullptr /* TODO */));
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::ALIAS IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::Alias::Construct (static)
//
// Constructs a new Alias instance
//
// Arguments:
//
//	name		- Name to assign to this Alias instance
//	parent		- Parent alias for this Alias instance, or nullptr if this is root
//	node		- Node instance to be referenced by this Alias instance

std::shared_ptr<HostFileSystem::Alias> HostFileSystem::Alias::Construct(const uapi::char_t* name, 
	const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node)
{
	_ASSERTE(name);
	_ASSERTE(node);

	// Every Alias must have a name and a reference to a node object
	if((!name) || (*name == 0) || (!node)) throw LinuxException(LINUX_EINVAL);

	// Construct a new shared Alias instance and return it to the caller
	return std::make_shared<Alias>(name, parent, node);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Alias::getParent
//
// Accesses the parent alias for this alias instance

FileSystem::AliasPtr HostFileSystem::Alias::getParent(void)
{
	// The parent is stored as a weak reference that must be converted
	FileSystem::AliasPtr parent = m_parent.lock();
	if(!parent) throw LinuxException(LINUX_ENOENT);

	return parent;
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::DIRECTORYNODE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::DIRECTORYNODE::HANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::FILENODE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::FILENODE::EXECHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::FILENODE::HANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::MOUNTPOINT IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::MountPoint Constructor
//
// Arguments:
//
//	handle		- Handle to the host directory object
//	flags		- Standard mounting flags
//	data		- Optional custom mounting flags and data

HostFileSystem::MountPoint::MountPoint(HANDLE handle, uint32_t flags, const void* data) : m_handle(handle), m_options(flags, data)
{
	// Determine the amount of space that needs to be allocated for the canonicalized path name string; when 
	// providing NULL for the output, this will include the count for the NULL terminator
	uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw MapHostException();

	// Retrieve the canonicalized path to the directory object based on the handle
	m_hostpath.resize(pathlen);
	pathlen = GetFinalPathNameByHandle(handle, m_hostpath.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw MapHostException();
}

//-----------------------------------------------------------------------------
// HostFileSystem::MountPoint Destructor

HostFileSystem::MountPoint::~MountPoint()
{
	// Close the operating system handle that references the mount point
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::PATHHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#pragma warning(pop)
