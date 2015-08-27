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
#include "HostFileSystem.h"

#include <Shlwapi.h>
#include <linux/fs.h>
#include <linux/magic.h>
#include <linux/time.h>
#include "Capability.h"
#include "MountOptions.h"
#include "LinuxException.h"
#include "SystemInformation.h"
#include "Win32Exception.h"
#include "convert.h"

#pragma comment(lib, "shlwapi.lib")

#pragma warning(push, 4)

// Invariants
//
static_assert(FILE_BEGIN == LINUX_SEEK_SET,		"HostFileSystem: FILE_BEGIN must be the same value as LINUX_SEEK_SET");
static_assert(FILE_CURRENT == LINUX_SEEK_CUR,	"HostFileSystem: FILE_CURRENT must be the same value as LINUX_SEEK_CUR");
static_assert(FILE_END == LINUX_SEEK_END,		"HostFileSystem: FILE_END must be the same value as LINUX_SEEK_END");

// Local Function Prototypes
//
static windows_path		HandleToPathW(HANDLE handle);
static LinuxException	MapHostException(DWORD code);

//-----------------------------------------------------------------------------
// HandleAccessToHostAccess (local)
//
// Converts FileSystem::HandleAccess to Windows access mode flags
//
// Arguments:
//
//	access		- FileSystem::HandleAccess to be converted
//	flags		- Linux fnctl flags to be converted

inline static DWORD HandleAccessToHostAccess(FileSystem::HandleAccess access)
{
	// Convert the HandleAccess bitmask value into compatible host flags
	if(access == FileSystem::HandleAccess::ReadOnly) return FILE_GENERIC_READ;
	else if(access == FileSystem::HandleAccess::WriteOnly) return FILE_GENERIC_WRITE;
	else if(access == FileSystem::HandleAccess::ReadWrite) return FILE_GENERIC_READ | FILE_GENERIC_WRITE;
	else throw LinuxException(LINUX_EINVAL);
}

//-----------------------------------------------------------------------------
// HandleToPathW (local)
//
// Gets the normalized path for a Windows file system handle
//
// Arguments:
//
//	handle		- Windows file system handle to get the path for

static windows_path HandleToPathW(HANDLE handle)
{
	std::unique_ptr<wchar_t[]>	path;					// Normalized file system path
	DWORD						cch, pathlen = 0;		// Path string lengths

	_ASSERTE((handle) && (handle != INVALID_HANDLE_VALUE));
	if((handle == nullptr) || (handle == INVALID_HANDLE_VALUE)) throw LinuxException(LINUX_EINVAL);

	// There is a possibility that the file system object could be renamed externally between calls to
	// GetFinalPathNameByHandle so this must be done in a loop to ensure that it ultimately succeeds
	do {

		// If the buffer is too small, this will return the required size including the null terminator
		// otherwise it will return the number of characters copied into the output buffer
		cch = GetFinalPathNameByHandleW(handle, path.get(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if(cch == 0) throw MapHostException(GetLastError());

		if(cch > pathlen) {

			// The buffer is too small for the current object path, reallocate it
			path = std::make_unique<wchar_t[]>(pathlen = cch);
			if(!path) throw LinuxException(LINUX_ENOMEM);
		}

	} while(cch >= pathlen);

	// Use (cch + 1) as the path buffer length rather than pathlen here, if the
	// object was renamed the buffer may actually be longer than necessary
	return windows_path{ std::move(path), cch + 1 };
}

//-----------------------------------------------------------------------------
// MapHostException (local)
//
// Converts a Win32 error code into a representative LinuxException instance
//
// Arguments:
//
//	code		- Win32 error code to be mapped

static LinuxException MapHostException(DWORD code)
{
	int linuxcode = LINUX_EIO;		// Use EIO as the default linux error code

	// Try to map the Win32 error code to something that makes sense
	switch(code) {

		case ERROR_ACCESS_DENIED:		linuxcode = LINUX_EACCES; break;
		case ERROR_FILE_NOT_FOUND:		linuxcode = LINUX_ENOENT; break;
		case ERROR_PATH_NOT_FOUND:		linuxcode = LINUX_ENOENT; break;
		case ERROR_FILE_EXISTS:			linuxcode = LINUX_EEXIST; break;
		case ERROR_INVALID_PARAMETER:	linuxcode = LINUX_EINVAL; break;
		case ERROR_ALREADY_EXISTS:		linuxcode = LINUX_EEXIST; break;
		case ERROR_NOT_ENOUGH_MEMORY:	linuxcode = LINUX_ENOMEM; break;
	}

	// Generate a LinuxException with the mapped code and provide the underlying Win32
	// error as an inner Win32Exception instance
	return LinuxException(linuxcode, Win32Exception(code));
}

//-----------------------------------------------------------------------------
// HostFileSystem Constructor (private)
//
// Arguments:
//
//	source		- Source string provided to mount function
//	flags		- File system specific mounting flags

HostFileSystem::HostFileSystem(const char_t* source, uint32_t flags) : m_source(source), m_flags(flags), m_fsid(FileSystem::GenerateFileSystemId())
{
	// No mount-specific flags should be specified for the file system instance
	_ASSERTE((flags & LINUX_MS_PERMOUNT_MASK) == 0);
}

//-----------------------------------------------------------------------------
// HostFileSystem::CreateDirectoryNode (private, static)
//
// Creates a new DirectoryNode instance from a host file system object
//
// Arguments:
//
//	fs			- Parent file system instance
//	path		- Native operating system path

std::shared_ptr<HostFileSystem::DirectoryNode> HostFileSystem::CreateDirectoryNode(std::shared_ptr<HostFileSystem> fs, const windows_path& path)
{
	std::shared_ptr<DirectoryNode>			node;				// The new node instance
	FILE_BASIC_INFO							info;				// Basic file information

	// Attempt to open a query-only handle against the file system object (don't use FILE_GENERIC_EXECUTE for directories)
	HANDLE handle = ::CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException(GetLastError());

	try {

		// Get the basic information about the handle to ensure that it represents a directory node
		if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &info, sizeof(FILE_BASIC_INFO))) throw MapHostException(GetLastError());
		if((info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);

		// Construct the node instance; this will take ownership of the operating system handle
		node = std::make_shared<DirectoryNode>(fs, handle);
	}

	catch(...) { CloseHandle(handle); throw; }

	// Ensure that the final normalized path to the node is within the virtual file system [sandbox]
	if(wcsncmp(fs->m_sandbox.c_str(), node->NormalizedPath, fs->m_sandbox.length()) != 0) throw LinuxException(LINUX_EXDEV);

	// Place a weak reference to the node into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ fs->m_cs };
	if(!fs->m_nodes.emplace(node.get(), node).second) throw LinuxException(LINUX_ENOMEM);
	
	return node;
}

//-----------------------------------------------------------------------------
// HostFileSystem::CreateFileNode (private, static)
//
// Creates a new FileNode instance from a host file system object
//
// Arguments:
//
//	fs			- Parent file system instance
//	disposition	- Operation disposition flags
//	path		- Native operating system path

std::shared_ptr<HostFileSystem::FileNode> HostFileSystem::CreateFileNode(std::shared_ptr<HostFileSystem> fs, DWORD disposition, const windows_path& path)
{
	std::shared_ptr<FileNode>				node;				// The new node instance
	FILE_BASIC_INFO							info;				// Basic file information

	// Attempt to open a query-only handle against the file system object
	HANDLE handle = ::CreateFileW(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, disposition, FILE_FLAG_POSIX_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException(GetLastError());

	try {

		// Get the basic information about the handle to ensure that it does not represent a directory node
		if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &info, sizeof(FILE_BASIC_INFO))) throw MapHostException(GetLastError());
		if((info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) throw LinuxException(LINUX_EISDIR);

		// Construct the node instance; this will take ownership of the operating system handle
		node = std::make_shared<FileNode>(fs, handle);
	}

	catch(...) { CloseHandle(handle); throw; }

	// Ensure that the final normalized path to the node is within the virtual file system [sandbox]
	if(wcsncmp(fs->m_sandbox.c_str(), node->NormalizedPath, fs->m_sandbox.length()) != 0) throw LinuxException(LINUX_EXDEV);

	// Place a weak reference to the node into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ fs->m_cs };
	if(!fs->m_nodes.emplace(node.get(), node).second) throw LinuxException(LINUX_ENOMEM);

	return node;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount (static)
//
// Creates an instance of the file system
//
// Arguments:
//
//	source		- Source device path
//	flags		- Standard mount options bitmask
//	data		- Extended mount options data
//	datalength	- Length of the extended mount options data in bytes

std::shared_ptr<FileSystem::Mount> HostFileSystem::Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength)
{
	bool sandbox = true;						// Flag to sandbox the virtual file system

	if(source == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	Capability::Demand(Capability::SystemAdmin);

	// Parse the provided mounting options
	MountOptions options(flags, data, datalength);

	// Break up the standard mounting options bitmask into file system and mount specific masks
	auto fsflags = options.Flags & (LINUX_MS_RDONLY | LINUX_MS_SYNCHRONOUS | LINUX_MS_DIRSYNC | LINUX_MS_KERNMOUNT);
	auto mountflags = (options.Flags & LINUX_MS_NOEXEC) | LINUX_MS_NODEV | LINUX_MS_NOSUID;

	try {

		// sandbox
		//
		// Sets the option to check all nodes are within the base mounting path
		if(options.Arguments.Contains("sandbox")) sandbox = true;

		// nosandbox
		//
		// Clears the option to check all nodes are within the base mounting path
		if(options.Arguments.Contains("nosandbox")) sandbox = false;
	}

	catch(...) { throw LinuxException(LINUX_EINVAL); }

	// Construct the file system instance and the root directory node instance.  If the target
	// object is not a directory, CreateDirectoryNode() will throw ENOTDIR
	auto fs = std::make_shared<HostFileSystem>(source, fsflags);
	auto rootdir = CreateDirectoryNode(fs, windows_path(std::to_wstring(source).c_str()));
	
	// Assign the base path string from the normalized root node path to create the sandbox
	if(sandbox) fs->m_sandbox = rootdir->NormalizedPath;

	// Construct and return the mount instance, using the mount-specific flags
	return std::make_shared<class Mount>(fs, rootdir, mountflags);
}

//
// HOSTFILESYSTEM::ALIAS
//

//-----------------------------------------------------------------------------
// HostFileSystem::Alias Constructor
//
// Arguments:
//
//	fs				- Parent file system instance
//	name			- Name to assign to this alias
//	node			- Node to attach to this alias

HostFileSystem::Alias::Alias(std::shared_ptr<HostFileSystem> fs, const char_t* name, std::shared_ptr<FileSystem::Node> node)
	: m_fs(std::move(fs)), m_name(name), m_node(std::move(node))
{
}

//-----------------------------------------------------------------------------
// HostFileSystem::Alias::GetName
//
// Reads the name assigned to this alias
//
// Arguments:
//
//	buffer			- Output buffer
//	count			- Size of the output buffer, in bytes

uapi::size_t HostFileSystem::Alias::GetName(char_t* buffer, size_t count) const
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	// Copy the minimum of the name length or the output buffer size
	count = std::min(m_name.size(), count);
	memcpy(buffer, m_name.data(), count * sizeof(char_t));
	
	return count;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Alias::getName
//
// Gets the name assigned to this alias

std::string HostFileSystem::Alias::getName(void) const
{
	return std::string(m_name);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Alias::getNode
//
// Gets the node to which this alias refers

std::shared_ptr<FileSystem::Node> HostFileSystem::Alias::getNode(void) const
{
	return m_node;
}

//
// HOSTFILESYSTEM::DIRECTORYHANDLE
//

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::getAccess
//
// Gets the handle access mode

FileSystem::HandleAccess HostFileSystem::DirectoryHandle::getAccess(void) const
{
	return m_access;
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Duplicate
//
// Creates a duplicate Handle instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Handle> HostFileSystem::DirectoryHandle::Duplicate(void) const
{
	HANDLE								duplicate;		// Duplicated operating system handle
	std::shared_ptr<DirectoryHandle>	handle;			// New DirectoryHandle object instance

	// Duplicate the existing operating system handle with the same flags and access as the contained one
	if(!DuplicateHandle(GetCurrentProcess(), m_handle, GetCurrentProcess(), &duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS))
		throw MapHostException(GetLastError());

	// Create the DirectoryHandle that will own the native operating system handle
	try { handle = std::make_shared<DirectoryHandle>(m_fs, duplicate, m_access, m_flags); }
	catch(...) { CloseHandle(duplicate); throw; }

	// Place a weak reference to the handle into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(!m_fs->m_handles.emplace(handle.get(), handle).second) throw LinuxException(LINUX_ENOMEM);

	return handle;
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::getFlags
//
// Gets the handle flags

FileSystem::HandleFlags HostFileSystem::DirectoryHandle::getFlags(void) const
{
	return m_flags;
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Destination memory buffer
//	count		- Size of the destination buffer, in bytes

uapi::size_t HostFileSystem::DirectoryHandle::Read(void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::ReadAt
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	offset		- Offset from the start of the node data to begin reading
//	buffer		- Destination memory buffer
//	count		- Size of the destination buffer, in bytes

uapi::size_t HostFileSystem::DirectoryHandle::ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);
	
	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Seek
//
// Changes the file position
//
// Arguments:
//
//	offset		- Offset into the node data to seek, relative to whence
//	whence		- Position in the node data that offset is relative to

uapi::loff_t HostFileSystem::DirectoryHandle::Seek(uapi::loff_t offset, int whence)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(whence);

	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments :
//
//	NONE

void HostFileSystem::DirectoryHandle::Sync(void) const
{
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void HostFileSystem::DirectoryHandle::SyncData(void) const
{
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer		- Source memory buffer
//	count		- Size of the source buffer, in bytes

uapi::size_t HostFileSystem::DirectoryHandle::Write(const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::WriteAt
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	offset		- Offset from the start of the node data to begin writing
//	buffer		- Source memory buffer
//	count		- Size of the source buffer, in bytes

uapi::size_t HostFileSystem::DirectoryHandle::WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	// This operation is invalid, the error to throw depends on O_PATH in the flags
	throw LinuxException(LINUX_EISDIR);
}

//
// HOSTFILESYSTEM::DIRECTORYNODE
//
		
//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::CreateDirectory
//
// Creates a new directory node as a child of this directory
//
// Arguments:
//
//	mount		- Mount on which this directory was reached
//	name		- Name to assign to the new node
//	mode		- Mode to assign to the new node

std::shared_ptr<FileSystem::Alias> HostFileSystem::DirectoryNode::CreateDirectory(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode)
{
	// The provided mount should always be an instance of HostFileSystem::Mount
	_ASSERTE(std::dynamic_pointer_cast<class Mount>(mount));

	// Directories cannot be created on read-only file systems
	if(mount->Flags & LINUX_MS_RDONLY) throw LinuxException(LINUX_EROFS);

	// todo: mode to SECURITY_ATTRIBUTES
	(mode);

	// Combine the new directory name with the current directory path and create it
	auto path = m_path.append(name);
	if(!::CreateDirectoryW(path, nullptr)) throw MapHostException(GetLastError());

	// There is a possibility that the directory could be modified or deleted between
	// the call to CreateDirectory() and opening the handle, remove it on exception
	try { return std::make_shared<Alias>(m_fs, name, CreateDirectoryNode(m_fs, path)); }
	catch(...) { RemoveDirectoryW(path); throw;  }
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::CreateFile
//
// Creates a new regular file node as a child of this directory
//
// Arguments:
//
//	mount		- Mount on which this directory was reached
//	name		- Name to assign to the new node
//	mode		- Mode to assign to the new node

std::shared_ptr<FileSystem::Alias> HostFileSystem::DirectoryNode::CreateFile(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode)
{
	// The provided mount should always be an instance of HostFileSystem::Mount
	_ASSERTE(std::dynamic_pointer_cast<class Mount>(mount));

	// Files cannot be created on read-only file systems
	if(mount->Flags & LINUX_MS_RDONLY) throw LinuxException(LINUX_EROFS);

	// todo: mode to SECURITY_ATTRIBUTES
	(mode);

	// Wrap a new FileNode instance into an Alias to return to the caller
	return std::make_shared<Alias>(m_fs, name, CreateFileNode(m_fs, CREATE_NEW, m_path.append(name)));
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::Lookup
//
// Looks up the alias associated with a child of this node
//
// Arguments:
//
//	mount		- Mount on which this directory was reached
//	name		- Name of the child alias to look up

std::shared_ptr<FileSystem::Alias> HostFileSystem::DirectoryNode::Lookup(std::shared_ptr<FileSystem::Mount> mount, const char_t* name) const
{
	UNREFERENCED_PARAMETER(mount);

	if(name == nullptr) throw LinuxException(LINUX_EFAULT);

	// Append the requested file system object name to the normalized directory path
	auto path = m_path.append(name);
	
	// Determine if the object exists and what kind of node needs to be created
	DWORD attributes = GetFileAttributes(path);
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOENT);

	// Currently this will create a new node for every lookup and not use any cached node
	// that may already represent the host file system object; this should be optimized
	std::shared_ptr<FileSystem::Node> node;
	if((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) node = CreateDirectoryNode(m_fs, path);
	else node = CreateFileNode(m_fs, OPEN_EXISTING, path);

	return std::make_shared<Alias>(m_fs, name, node);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::Open
//
// Creates a Handle instance against this node
//
// Arguments:
//
//	mount		- Mount on which this node was resolved
//	access		- Handle access mode
//	flags		- Handle flags

std::shared_ptr<FileSystem::Handle> HostFileSystem::DirectoryNode::Open(std::shared_ptr<FileSystem::Mount> mount, FileSystem::HandleAccess access, FileSystem::HandleFlags flags)
{
	HANDLE								duplicate;		// Duplicated operating system handle
	std::shared_ptr<DirectoryHandle>	handle;			// New DirectoryHandle object instance

	UNREFERENCED_PARAMETER(mount);
	_ASSERTE(std::dynamic_pointer_cast<class Mount>(mount));

	// Directory handles must always be opened in read-only mode
	if(access != FileSystem::HandleAccess::ReadOnly) throw LinuxException(LINUX_EISDIR);

	// Check for flags that are incompatible with opening a directory file system object
	if(flags & (FileSystem::HandleFlags::Append | FileSystem::HandleFlags::Direct)) throw LinuxException(LINUX_EINVAL);

	// Duplicate the original query-only file system object handle (don't ask for FILE_GENERIC_READ on directories)
	if(!DuplicateHandle(GetCurrentProcess(), m_handle, GetCurrentProcess(), &duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS)) 
		throw MapHostException(GetLastError());

	// Create the DirectoryHandle that will own the native operating system handle
	try { handle = std::make_shared<DirectoryHandle>(m_fs, duplicate, access, flags); }
	catch(...) { CloseHandle(duplicate); throw; }

	// Place a weak reference to the handle into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(!m_fs->m_handles.emplace(handle.get(), handle).second) throw LinuxException(LINUX_ENOMEM);

	return handle;
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::SetOwnership
//
// Changes the ownership of this node
//
// Arguments:
//
//	uid			- New ownership user identifier
//	gid			- New ownership group identifier

void HostFileSystem::DirectoryNode::SetOwnership(uapi::uid_t uid, uapi::gid_t gid)
{
	UNREFERENCED_PARAMETER(uid);
	UNREFERENCED_PARAMETER(gid);

	// Node ownership is controlled by the host operating system
	throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::SetPermissions
//
// Changes the permission flags for this node
//
// Arguments:
//
//	permissions		- New permission flags for the node

void HostFileSystem::DirectoryNode::SetPermissions(uapi::mode_t permissions)
{
	UNREFERENCED_PARAMETER(permissions);

	// Node permissions are controlled by the host operating system
	throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::Stat
//
// Provides statistical information about the node
//
// Arguments:
//
//	stats		- Buffer to receive the node statistics

void HostFileSystem::DirectoryNode::Stat(uapi::stat* stats) const
{
	BY_HANDLE_FILE_INFORMATION		info;		// File information
	FILE_STORAGE_INFO				storage;	// Storage information

	if(stats == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// The bulk of the information needed is provided in BY_HANDLE_FILE_INFORMATION
	if(!GetFileInformationByHandle(m_handle, &info)) throw MapHostException(GetLastError());
	
	// Retrieve the FILE_STORAGE_INFO for the to determine the performance block size
	if(!GetFileInformationByHandleEx(m_handle, FileStorageInfo, &storage, sizeof(FILE_STORAGE_INFO)))
		throw LinuxException(MapHostException(GetLastError()));

	memset(stats, 0, sizeof(uapi::stat));

	//stats->st_dev		= (0 << 16) | 0;			// TODO
	stats->st_ino		= static_cast<uint64_t>(info.nFileIndexHigh) << 32 | info.nFileIndexLow;
	stats->st_nlink		= 2 + 0;					// TODO
	//stats->st_mode	= 0;						// TODO
	//stats->st_uid		= 0;						// TODO
	//stats->st_gid		= 0;						// TODO
	//stats->st_rdev	= (0 << 16) | 0;			// TODO
	stats->st_blksize	= storage.PhysicalBytesPerSectorForPerformance;
	stats->st_atime		= convert<uapi::timespec>(info.ftLastAccessTime);
	stats->st_mtime		= convert<uapi::timespec>(info.ftLastWriteTime);
	stats->st_ctime		= convert<uapi::timespec>(info.ftCreationTime);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::getType
//
// Gets the type of file system node being implemented

FileSystem::NodeType HostFileSystem::DirectoryNode::getType(void) const
{
	return FileSystem::NodeType::Directory;
}

//
// HOSTFILESYSTEM::FILEHANDLE
//

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::getAccess
//
// Gets the handle access mode

FileSystem::HandleAccess HostFileSystem::FileHandle::getAccess(void) const
{
	return m_access;
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Duplicate
//
// Creates a duplicate Handle instance
//
// Arguments:
//
//	flags		- Flags for the duplicated handle

std::shared_ptr<FileSystem::Handle> HostFileSystem::FileHandle::Duplicate(void) const
{
	HANDLE								duplicate;		// Duplicated operating system handle
	std::shared_ptr<FileHandle>			handle;			// New FileHandle object instance

	// Duplicate the existing operating system handle with the same access
	if(!DuplicateHandle(GetCurrentProcess(), m_handle, GetCurrentProcess(), &duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS))
		throw MapHostException(GetLastError());

	// Create the FileHandle that will own the native operating system handle
	try { handle = std::make_shared<FileHandle>(m_fs, duplicate, m_access, m_flags); }
	catch(...) { CloseHandle(duplicate); throw; }

	// Place a weak reference to the handle into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(!m_fs->m_handles.emplace(handle.get(), handle).second) throw LinuxException(LINUX_ENOMEM);

	return handle;
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::getFlags
//
// Gets the handle flags

FileSystem::HandleFlags HostFileSystem::FileHandle::getFlags(void) const
{
	return m_flags;
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Destination memory buffer
//	count		- Size of the destination buffer, in bytes

uapi::size_t HostFileSystem::FileHandle::Read(void* buffer, uapi::size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	// ReadFile() can only read up to MAXDWORD bytes from the underlying file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL);

	// Attempt to read the specified number of bytes from the file into the buffer
	DWORD read = static_cast<DWORD>(count);
	if(!ReadFile(m_handle, buffer, read, &read, nullptr)) throw MapHostException(GetLastError());

	return static_cast<uapi::size_t>(read);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::ReadAt
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	offset		- Offset from the start of the node data to begin reading
//	buffer		- Destination memory buffer
//	count		- Size of the destination buffer, in bytes

uapi::size_t HostFileSystem::FileHandle::ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	// ReadFile() can only read up to MAXDWORD bytes from the underlying file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL);

	// OVERLAPPED structure can be used to read from a specific position
	OVERLAPPED overlapped = { 0 };
	overlapped.Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
	overlapped.OffsetHigh = (offset >> 32);

	// Attempt to read the specified number of bytes from the file into the buffer
	DWORD read = static_cast<DWORD>(count);
	if(!ReadFile(m_handle, buffer, read, &read, &overlapped)) throw MapHostException(GetLastError());

	return static_cast<uapi::size_t>(read);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Seek
//
// Changes the file position
//
// Arguments:
//
//	offset		- Offset into the node data to seek, relative to whence
//	whence		- Position in the node data that offset is relative to

uapi::loff_t HostFileSystem::FileHandle::Seek(uapi::loff_t offset, int whence)
{
	LARGE_INTEGER			position;			// Distance to be moved

	// This is a straightforward operation provided the values for whence mean the same
	// things, which is checked as an invariant at the top of this file
	position.QuadPart = offset;
	if(!SetFilePointerEx(m_handle, position, &position, whence)) throw MapHostException(GetLastError());

	return position.QuadPart;
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments :
//
//	NONE

void HostFileSystem::FileHandle::Sync(void) const
{
	// Read-only file systems prevent synchronization of the file data
	if(m_fs->m_flags & LINUX_MS_RDONLY) throw LinuxException(LINUX_EROFS);

	// The closest equivalent for this operation is FlushFileBuffers()
	if(!FlushFileBuffers(m_handle)) throw MapHostException(GetLastError());
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void HostFileSystem::FileHandle::SyncData(void) const
{
	Sync();		// This is the same operation as Sync() for HostFileSystem
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer		- Source memory buffer
//	count		- Size of the source buffer, in bytes

uapi::size_t HostFileSystem::FileHandle::Write(const void* buffer, uapi::size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	// Attempting to write to a read-only handle yields EINVAL, not EACCES
	if(m_access == FileSystem::HandleAccess::ReadOnly) throw LinuxException(LINUX_EINVAL);

	// Attempting to write to a read-only file system yields EROFS, not EACCES
	if(m_fs->m_flags & LINUX_MS_RDONLY) throw LinuxException(LINUX_EROFS);

	// WriteFile() can only write up to MAXDWORD bytes into the target file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL);

	// Attempt to write the specified number of bytes from the buffer into the file
	DWORD written = static_cast<DWORD>(count);
	if(!WriteFile(m_handle, buffer, written, &written, nullptr)) throw MapHostException(GetLastError());

	// MS_SYNCHRONOUS can be set on the mount to imply O_SYNC for all handles
	if((m_fs->m_flags & LINUX_MS_SYNCHRONOUS) || (m_flags & FileSystem::HandleFlags::Sync)) FlushFileBuffers(m_handle);

	return static_cast<uapi::size_t>(written);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::WriteAt
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	offset		- Offset from the start of the node data to begin writing
//	buffer		- Source memory buffer
//	count		- Size of the source buffer, in bytes

uapi::size_t HostFileSystem::FileHandle::WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	// Attempting to write to a read-only handle yields EINVAL, not EACCES
	if(m_access == FileSystem::HandleAccess::ReadOnly) throw LinuxException(LINUX_EINVAL);

	// Attempting to write to a read-only file system yields EROFS, not EACCES
	if(m_fs->m_flags & LINUX_MS_RDONLY) throw LinuxException(LINUX_EROFS);

	// Data cannot be written to a specific location in O_APPEND mode
	if(m_flags & FileSystem::HandleFlags::Append) throw LinuxException(LINUX_EINVAL);

	// WriteFile() can only write up to MAXDWORD bytes to the underlying file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL);

	// OVERLAPPED structure can be used to write to a specific position
	OVERLAPPED overlapped = { 0 };
	overlapped.Offset = static_cast<DWORD>(offset & 0xFFFFFFFF);
	overlapped.OffsetHigh = (offset >> 32);

	// Attempt to write the specified number of bytes from the buffer into the file
	DWORD written = static_cast<DWORD>(count);
	if(!WriteFile(m_handle, buffer, written, &written, &overlapped)) throw MapHostException(GetLastError());

	// MS_SYNCHRONOUS can be set on the mount to imply O_SYNC for all handles.
	if((m_fs->m_flags & LINUX_MS_SYNCHRONOUS) || (m_flags & FileSystem::HandleFlags::Sync)) FlushFileBuffers(m_handle);

	return static_cast<uapi::size_t>(written);
}

//
// HOSTFILESYSTEM::FILENODE
//

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::Open
//
// Creates a Handle instance against this node
//
// Arguments:
//
//	mount		- Mount on which this node was resolved
//	access		- Handle access mode
//	flags		- Handle flags

std::shared_ptr<FileSystem::Handle> HostFileSystem::FileNode::Open(std::shared_ptr<FileSystem::Mount> mount, FileSystem::HandleAccess access, FileSystem::HandleFlags flags)
{
	std::shared_ptr<FileHandle>			handle;				// New FileHandle object instance

	// The Mount object should be a HostFileSystem::Mount instance
	_ASSERTE(std::dynamic_pointer_cast<class Mount>(mount));

	// If the file system was mounted as read-only, write access cannot be granted
	if((mount->Flags & LINUX_MS_RDONLY) && (access != FileSystem::HandleAccess::ReadOnly)) throw LinuxException(LINUX_EROFS);

	// O_APPEND requires write access to the file object
	if((flags & FileSystem::HandleFlags::Append) && (access == FileSystem::HandleAccess::ReadOnly)) throw LinuxException(LINUX_EINVAL);

	// Generate the access mode mask for the duplicate handle
	DWORD hostaccess = HandleAccessToHostAccess(access);

	// Generate the native attributes for the operation based on the provided flags
	DWORD attributes = FILE_FLAG_POSIX_SEMANTICS;
	if(flags & FileSystem::HandleFlags::Sync) attributes |= FILE_FLAG_WRITE_THROUGH;

	// Reopen the native file handle with the requested attributes
	HANDLE duplicate = ReOpenFile(m_handle, hostaccess, FILE_SHARE_READ | FILE_SHARE_WRITE, attributes);
	if(duplicate == INVALID_HANDLE_VALUE) throw MapHostException(GetLastError());

	// It's not possible to handle O_TRUNC as part of the ReOpenFile request, do it afterwards (should be fine)
	if((flags & FileSystem::HandleFlags::Truncate) && (access != FileSystem::HandleAccess::ReadOnly)) SetEndOfFile(duplicate);

	// O_APPEND requires that FILE_WRITE_DATA be removed from the access mask; this can't be done in the access mask
	// provided to ReOpenFile() above since FILE_WRITE_DATA might have been needed for handling O_TRUNC
	if(flags & FileSystem::HandleFlags::Append) {
	
		// Note that DuplicateHandle will close the original handle regardless of success when asked, there
		// is no need to call CloseHandle() on that if this fails
		if(!DuplicateHandle(GetCurrentProcess(), duplicate, GetCurrentProcess(), &duplicate, hostaccess & ~FILE_WRITE_DATA, FALSE, DUPLICATE_CLOSE_SOURCE))
			throw LinuxException(MapHostException(GetLastError()));
	}

	// Create the FileHandle that will own the native operating system handle
	try { handle = std::make_shared<FileHandle>(m_fs, duplicate, access, flags); }
	catch(...) { CloseHandle(duplicate); throw; }

	// Place a weak reference to the handle into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(!m_fs->m_handles.emplace(handle.get(), handle).second) throw LinuxException(LINUX_ENOMEM);

	return handle;
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::OpenExec
//
// Creates an execute-only handle against this node
//
// Arguments:
//
//	mount		- Mount on which this node was resolved

std::shared_ptr<FileSystem::Handle> HostFileSystem::FileNode::OpenExec(std::shared_ptr<FileSystem::Mount> mount) const
{
	std::shared_ptr<FileHandle>			handle;				// New FileHandle object instance

	// The Mount object should be an instance of HostFileSystem::Mount
	_ASSERTE(std::dynamic_pointer_cast<class Mount>(mount));

	// Verify that the mount point allows for execution of binary files
	if(mount->Flags & LINUX_MS_NOEXEC) throw LinuxException(LINUX_ENOEXEC);

	// Reopen the native file handle with EXECUTE and READ access to the file
	HANDLE duplicate = ReOpenFile(m_handle, FILE_GENERIC_EXECUTE | FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_FLAG_POSIX_SEMANTICS);
	if(duplicate == INVALID_HANDLE_VALUE) throw MapHostException(GetLastError());

	// Create the FileHandle that will own the native operating system handle
	try { handle = std::make_shared<FileHandle>(m_fs, duplicate, FileSystem::HandleAccess::ReadOnly, FileSystem::HandleFlags::None); }
	catch(...) { CloseHandle(duplicate); throw; }

	// Place a weak reference to the handle into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(!m_fs->m_handles.emplace(handle.get(), handle).second) throw LinuxException(LINUX_ENOMEM);

	return handle;
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::SetOwnership
//
// Changes the ownership of this node
//
// Arguments:
//
//	uid			- New ownership user identifier
//	gid			- New ownership group identifier

void HostFileSystem::FileNode::SetOwnership(uapi::uid_t uid, uapi::gid_t gid)
{
	UNREFERENCED_PARAMETER(uid);
	UNREFERENCED_PARAMETER(gid);

	// Node ownership is controlled by the host operating system
	throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::SetPermissions
//
// Changes the permission flags for this node
//
// Arguments:
//
//	permissions		- New permission flags for the node

void HostFileSystem::FileNode::SetPermissions(uapi::mode_t permissions)
{
	UNREFERENCED_PARAMETER(permissions);

	// Node permissions are controlled by the host operating system
	throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::Stat
//
// Provides statistical information about the node
//
// Arguments:
//
//	stats		- Buffer to receive the node statistics

void HostFileSystem::FileNode::Stat(uapi::stat* stats) const
{
	BY_HANDLE_FILE_INFORMATION		info;		// File information
	FILE_STORAGE_INFO				storage;	// Storage information

	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	// The bulk of the information needed is provided in BY_HANDLE_FILE_INFORMATION
	if(!GetFileInformationByHandle(m_handle, &info)) throw MapHostException(GetLastError());

	// Retrieve the FILE_STORAGE_INFO for the to determine the performance block size
	if(!GetFileInformationByHandleEx(m_handle, FileStorageInfo, &storage, sizeof(FILE_STORAGE_INFO)))
		throw LinuxException(MapHostException(GetLastError()));

	memset(stats, 0, sizeof(uapi::stat));
	
	//stats->st_dev		= (0 << 16) | 0;			// TODO
	stats->st_ino		= static_cast<uint64_t>(info.nFileIndexHigh) << 32 | info.nFileIndexLow;
	stats->st_nlink		= info.nNumberOfLinks;
	//stats->st_mode	= 0;						// TODO
	//stats->st_uid		= 0;						// TODO
	//stats->st_gid		= 0;						// TODO
	//stats->st_rdev	= (0 << 16) | 0;			// TODO
	stats->st_size		= static_cast<uint64_t>(info.nFileSizeHigh) << 32 | info.nFileSizeLow;
	stats->st_blksize	= storage.PhysicalBytesPerSectorForPerformance;
	stats->st_blocks	= (stats->st_size + 511) / 512;
	stats->st_atime		= convert<uapi::timespec>(info.ftLastAccessTime);
	stats->st_mtime		= convert<uapi::timespec>(info.ftLastWriteTime);
	stats->st_ctime		= convert<uapi::timespec>(info.ftCreationTime);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::getType
//
// Gets the type of file system node being implemented

FileSystem::NodeType HostFileSystem::FileNode::getType(void) const
{
	return FileSystem::NodeType::File;
}

//
// HOSTFILESYSTEM::HANDLEBASE
//

//-----------------------------------------------------------------------------
// HostFileSystem::HandleBase Constructor
//
// Arguments:
//
//	fs			- Parent file system instance
//	handle		- Native operating system handle
//	access		- Handle access mode
//	flags		- Handle flags

HostFileSystem::HandleBase::HandleBase(std::shared_ptr<HostFileSystem> fs, HANDLE handle, FileSystem::HandleAccess access, FileSystem::HandleFlags flags) 
	: m_fs(std::move(fs)), m_handle(handle), m_access(access), m_flags(flags)
{
	if((handle == nullptr) || (handle == INVALID_HANDLE_VALUE)) throw Win32Exception(ERROR_INVALID_HANDLE);
}

//-----------------------------------------------------------------------------
// HostFileSystem::HandleBase Destructor

HostFileSystem::HandleBase::~HandleBase() 
{
	// Remove this handle instance from the file system's tracking collection
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	m_fs->m_handles.erase(this);

	// Close the operating system handle
	CloseHandle(m_handle);
}

//
// HOSTFILESYSTEM::MOUNT
//

//-----------------------------------------------------------------------------
// HostFileSystem::Mount Constructor (private)
//
// Arguments:
//
//	fs		- Reference to the HostFileSystem instance
//	root	- Root directory node instance
//	flags	- Per-mount flags and options to set on this mount instance

HostFileSystem::Mount::Mount(std::shared_ptr<HostFileSystem> fs, std::shared_ptr<DirectoryNode> root, uint32_t flags) 
	: m_fs(std::move(fs)), m_root(std::move(root)), m_flags(flags)
{
	// The flags should only contain bits from MS_PERMOUNT_MASK
	_ASSERTE((m_flags & ~LINUX_MS_PERMOUNT_MASK) == 0);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount::Duplicate
//
// Duplicates this mount instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Mount> HostFileSystem::Mount::Duplicate(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	// Clone the underlying file system reference and flags into a new mount
	return std::make_shared<Mount>(m_fs, root, m_flags);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount::getFlags
//
// Gets the flags set on this mount, includes file system flags

uint32_t HostFileSystem::Mount::getFlags(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	return m_flags | m_fs->m_flags;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount::Remount
//
// Remounts the file system with different options
//
// Arguments:
//
//	flags		- Standard mounting option flags
//	data		- Extended/custom mounting options
//	datalength	- Length of the extended mounting options data

void HostFileSystem::Mount::Remount(uint32_t flags, const void* data, size_t datalen)
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	Capability::Demand(Capability::SystemAdmin);

	// MS_REMOUNT must be specified in the flags when calling this function
	if((flags & LINUX_MS_REMOUNT) != LINUX_MS_REMOUNT) throw LinuxException(LINUX_EINVAL);

	// Parse the provided mounting options into remount flags and key/value pairs
	MountOptions options(flags & LINUX_MS_RMT_MASK, data, datalen);

	// HostFileSystem::m_flags is atomic, grab the value
	uint32_t currentflags = static_cast<uint32_t>(m_fs->m_flags);

	// Filter the flags to only those options which have changed from the current ones
	uint32_t changedflags = currentflags ^ options.Flags;

	// MS_RDONLY
	//
	if(changedflags & LINUX_MS_RDONLY)
		currentflags = (currentflags & ~LINUX_MS_RDONLY) | options[LINUX_MS_RDONLY];

	// MS_SYNCHRONOUS
	//
	if(changedflags & LINUX_MS_SYNCHRONOUS)
		currentflags = (currentflags & ~LINUX_MS_SYNCHRONOUS) | options[LINUX_MS_SYNCHRONOUS];

	// Apply the updated bitmask to the file system flags
	m_fs->m_flags = currentflags;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount::getRoot
//
// Gets a reference to the root directory of the mount point

std::shared_ptr<FileSystem::Directory> HostFileSystem::Mount::getRoot(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	return root;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount::getSource
//
// Gets the device/name used as the source of the file system

std::string HostFileSystem::Mount::getSource(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	return std::string(m_fs->m_source);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount::Stat
//
// Provides statistical information about the mounted file system
//
// Arguments:
//
//	stats		- Structure to receieve the file system statistics

void HostFileSystem::Mount::Stat(uapi::statfs* stats) const
{
	FILE_STORAGE_INFO		info;			// FILE_STORAGE_INFO about the root node
	ULARGE_INTEGER			available;		// Volume bytes available to the caller
	ULARGE_INTEGER			total;			// Total volume size in bytes
	ULARGE_INTEGER			free;			// Total volume bytes available

	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	// Retrieve the FILE_STORAGE_INFO for the root directory node to determine the block sizes
	if(!GetFileInformationByHandleEx(m_root->Handle, FileStorageInfo, &info, sizeof(FILE_STORAGE_INFO)))
		throw LinuxException(MapHostException(GetLastError()));
	
	// Retrieve the free space statistics for the volume on which the root directory exists
	if(!GetDiskFreeSpaceEx(m_root->NormalizedPath, &available, &total, &free)) 
		throw LinuxException(MapHostException(GetLastError()));

	stats->f_type		= LINUX_HOSTFS_SUPER_MAGIC;
	stats->f_bsize		= info.PhysicalBytesPerSectorForPerformance;
	stats->f_blocks		= total.QuadPart / info.LogicalBytesPerSector;
	stats->f_bfree		= free.QuadPart / info.LogicalBytesPerSector;
	stats->f_bavail		= available.QuadPart / info.LogicalBytesPerSector;
	stats->f_files		= 0;
	stats->f_ffree		= 0;
	stats->f_fsid		= m_fs->m_fsid;
	stats->f_namelen	= MAX_PATH;
	stats->f_frsize		= info.LogicalBytesPerSector;
	stats->f_flags		= m_flags | m_fs->m_flags;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount::Unmount
//
// Unmounts the file system
//
// Arguments:
//
//	NONE

void HostFileSystem::Mount::Unmount(void)
{
	// There can only be one active node (m_root) and no active handles
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if((m_fs->m_nodes.size() > 1) || (m_fs->m_handles.size() > 0)) throw LinuxException(LINUX_EBUSY);

	// Ensure that the root directory node is also not shared out
	if(m_root.use_count() > 1) throw LinuxException(LINUX_EBUSY);

	m_root.reset();			// Release the root directory node
}

//
// HOSTFILESYSTEM::NODEBASE
//

//-----------------------------------------------------------------------------
// HostFileSystem::NodeBase Constructor
//
// Arguments:
//
//	fs				- Parent file system instance
//	handle			- Native operating system handle

HostFileSystem::NodeBase::NodeBase(std::shared_ptr<HostFileSystem> fs, HANDLE handle) 
	: m_fs(std::move(fs)), m_handle(handle), m_path(HandleToPathW(handle))
{
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeBase Destructor

HostFileSystem::NodeBase::~NodeBase() 
{
	// Remove this node instance from the file system's tracking collection
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	m_fs->m_nodes.erase(this);

	// Close the operating system handle
	CloseHandle(m_handle);
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::NodeBase::getHandle
//
// Gets the underlying query-only file system object handle

HANDLE HostFileSystem::NodeBase::getHandle(void) const
{
	return m_handle;
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeBase::getNormalizedPath
//
// Gets the normalized path of this node on the host file system

const wchar_t* HostFileSystem::NodeBase::getNormalizedPath(void) const
{
	return m_path;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
