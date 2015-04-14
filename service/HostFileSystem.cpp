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

#pragma warning(push, 4)

// INVARIANTS
//
static_assert(FILE_BEGIN == LINUX_SEEK_SET,		"HostFileSystem: FILE_BEGIN must be the same value as LINUX_SEEK_SET");
static_assert(FILE_CURRENT == LINUX_SEEK_CUR,	"HostFileSystem: FILE_CURRENT must be the same value as LINUX_SEEK_CUR");
static_assert(FILE_END == LINUX_SEEK_END,		"HostFileSystem: FILE_END must be the same value as LINUX_SEEK_END");

////
// FROM OLD FILE -- NEED TO DO SOMETHING SIMILAR HERE
////

//////-----------------------------------------------------------------------------
////// HostFileSystem::MountPoint::ValidateHandle
//////
////// Validates that a newly opened file system handle meets the criteria set
////// for this host mount point
//////
////// Arguments:
//////
//////	handle		- Operating system handle to validate
////
////void HostFileSystem::MountPoint::ValidateHandle(HANDLE handle)
////{
////	// If path verification is active, get the canonicalized name associated with this
////	// handle and ensure that it is a child of the mounted root directory; this prevents
////	// access outside of the mountpoint by symbolic links or ".." parent path components
////	if(m_verifypath) {
////
////		// Determine the amount of space that needs to be allocated for the directory path name string; when 
////		// providing NULL for the output, this will include the count for the NULL terminator
////		uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
////		if(pathlen == 0) throw HostFileSystem::MapException();
////
////		// If the path is at least as long as the original mountpoint name, it cannot be a child
////		if(pathlen < m_path.size()) throw LinuxException(LINUX_EPERM);
////
////		// Retrieve the path to the directory object based on the handle
////		std::vector<tchar_t> path(pathlen);
////		pathlen = GetFinalPathNameByHandle(handle, path.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
////		if(pathlen == 0) throw HostFileSystem::MapException();
////
////		// Verify that the canonicalized path starts with the mount point, don't ignore case
////		if(_tcsncmp(m_path.data(), path.data(), m_path.size() - 1) != 0) throw LinuxException(LINUX_EPERM);
////	}
////}

//-----------------------------------------------------------------------------
// FlagsToAccess (local)
//
// Converts linux fnctl flags to Windows access mode flags for CreateFile
//
// Arguments:
//
//	flags		- Linux fnctl flags to be converted

static DWORD FlagsToAccess(int flags)
{
	// O_PATH does not define any read or write flags, it's just a handle
	if(flags & LINUX_O_PATH) return 0;

	// Convert the Linux read/write flags into compatible host flags
	switch(flags & LINUX_O_ACCMODE) {

		// O_RDONLY --> FILE_GENERIC_READ
		case LINUX_O_RDONLY: return FILE_GENERIC_READ;

		// O_WRONLY --> FILE_GENERIC_WRITE
		case LINUX_O_WRONLY: return FILE_GENERIC_WRITE;

		// O_RDWR --> FILE_GENERIC_READ | FILE_GENERIC_WRITE;
		case LINUX_O_RDWR: return FILE_GENERIC_READ | FILE_GENERIC_WRITE;
	}

	// Possible exception if both O_WRONLY (1) and O_RDWR (2) are set
	throw LinuxException(LINUX_EINVAL);
}

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
// HandleToIndex (local)
//
// Gets the file system object index from a Windows handle
//
// Arguments:
//
//	handle		- Windows file system handle to get the index for

static uint64_t HandleToIndex(HANDLE handle)
{
	BY_HANDLE_FILE_INFORMATION		fileinfo;		// File information

	_ASSERTE(handle != INVALID_HANDLE_VALUE);
	if(handle == INVALID_HANDLE_VALUE) return 0;

	// Query information about the object from the handle and return the file index
	if(!GetFileInformationByHandle(handle, &fileinfo)) throw MapHostException();
	return (static_cast<uint64_t>(fileinfo.nFileIndexHigh) << 32) | fileinfo.nFileIndexLow;
}

//-----------------------------------------------------------------------------
// HandleToPath (local)
//
// Gets the canonicalized path for a Windows file system handle
//
// Arguments:
//
//	handle		- Windows file system handle to get the path for

static HeapBuffer<tchar_t> HandleToPath(HANDLE handle)
{
	_ASSERTE(handle != INVALID_HANDLE_VALUE);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT);

	// Determine the amount of space that needs to be allocated for the canonicalized path name string; when 
	// providing NULL for the output, this will include the count for the NULL terminator
	uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw MapHostException();

	// Retrieve the canonicalized path to the directory object based on the handle
	HeapBuffer<tchar_t> hostpath(pathlen);
	pathlen = GetFinalPathNameByHandle(handle, &hostpath, pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw MapHostException();

	return hostpath;
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::DuplicateDirectoryHandle (static)
//
// Duplicates an existing directory handle object
//
// Arguments:
//
//	mountpoint	- MountPoint instance for the parent file system
//	alias		- Alias instance used when original handle was generated
//	handle		- Existing directory object handle
//	flags		- File operation flags and attributes

FileSystem::HandlePtr HostFileSystem::DuplicateDirectoryHandle(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<FileSystem::Alias>& alias,
	HANDLE handle, int flags)
{
	_ASSERTE(mountpoint);
	_ASSERTE(handle != INVALID_HANDLE_VALUE);

	// Directory node handles must be opened in read-only mode
	if((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY) throw LinuxException(LINUX_EISDIR);

	// Use the contained query-only handle to reopen the directory with read-only attributes
	HANDLE duplicate = ReOpenFile(handle, FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS);
	if(duplicate == INVALID_HANDLE_VALUE) throw MapHostException();

	try { 

		// Generate a new Handle instance around the new object handle
		if(flags & LINUX_O_PATH) return std::make_shared<PathHandle>(mountpoint, alias, duplicate, flags);
		else return std::make_shared<DirectoryHandle>(mountpoint, alias, duplicate, flags);
	}
	catch(...) { CloseHandle(duplicate); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::DuplicateFileHandle (static)
//
// Duplicates an existing file handle object
//
// Arguments:
//
//	mountpoint	- MountPoint instance for the parent file system
//	alias		- Alias instance used when original handle was generated
//	handle		- Existing directory object handle
//	flags		- File operation flags and attributes

FileSystem::HandlePtr HostFileSystem::DuplicateFileHandle(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<FileSystem::Alias>& alias,
	HANDLE handle, int flags)
{
	_ASSERTE(mountpoint);
	_ASSERTE(handle != INVALID_HANDLE_VALUE);

	// O_DIRECTORY verifies that the target node is a directory, which this is not
	if(flags & LINUX_O_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);

	// HostFileSystem does not support unnamed temporary files via O_TMPFILE
	// TODO: why is this here, this is Open(), not Create()
	if(flags & LINUX___O_TMPFILE) throw LinuxException(LINUX_EOPNOTSUPP);

	// If the file system was mounted as read-only, write access cannot be granted
	if(mountpoint->Options.ReadOnly && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) throw LinuxException(LINUX_EROFS);

	// Generate the attributes for the open operation based on the provided flags
	DWORD attributes = FILE_FLAG_POSIX_SEMANTICS;
	if(flags & LINUX_O_SYNC) attributes |= FILE_FLAG_WRITE_THROUGH;
	if(flags & LINUX_O_DIRECT) attributes |= FILE_FLAG_NO_BUFFERING;

	// Use the contained query-only handle to reopen the file with the requested attributes
	HANDLE duplicate = ReOpenFile(handle, FlagsToAccess(flags), FILE_SHARE_READ | FILE_SHARE_WRITE, attributes);
	if(duplicate == INVALID_HANDLE_VALUE) throw MapHostException();

	try {

		// By using ReOpenFile() rather than CreateFile() to generate the handle, there is no opportunity
		// to specify TRUNCATE_EXISTING in the disposition flags, it must be done after the fact
		if((flags & LINUX_O_TRUNC) && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) { if(!SetEndOfFile(duplicate)) throw MapHostException(); }

		// Generate a new Handle instance around the new object handle and the original flags
		if(flags & LINUX_O_PATH) return std::make_shared<PathHandle>(mountpoint, alias, duplicate, flags);
		else return std::make_shared<FileHandle>(mountpoint, alias, duplicate, flags);
	}
	
	catch(...) { CloseHandle(duplicate); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount (static)
//
// Mounts the file system on a host system directory object
//
// Arguments:
//
//	source		- Host path; must reference a directory object
//	flags		- Standard mounting options and flags
//	data		- Filesystem-specific mounting data
//	datalen		- Length of the filesystem specific mounting data

FileSystemPtr HostFileSystem::Mount(const uapi::char_t* source, uint32_t flags, const void* data, size_t datalen)
{
	std::shared_ptr<MountPoint>			mountpoint;			// MountPoint instance

	if(!source) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	std::tstring hostpath = std::to_tstring(source);		// Convert source from ANSI/UTF-8

	// Determine the type of node that the path represents; must be a directory for mounting
	DWORD attributes = GetFileAttributes(hostpath.c_str());
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOTDIR);
	if((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) throw LinuxException(LINUX_ENOTDIR);

	// Attempt to open a query-only handle against the file system directory object
	HANDLE handle = ::CreateFile(hostpath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 
		FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException();

	// Attempt to create the MountPoint instance for this file system against the opened handle
	// (MountPoint will take ownership of the handle at this point)
	try { mountpoint = std::make_shared<MountPoint>(handle, flags, data, datalen); }
	catch(...) { CloseHandle(handle); throw; }

	// Construct the HostFileSystem instance, providing an alias attached to the target directory
	auto rootnode = DirectoryNode::FromPath(mountpoint, mountpoint->HostPath);
	return std::make_shared<HostFileSystem>(mountpoint, Alias::Construct("", rootnode));
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
	if((!name) || (!node)) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

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
// HOSTFILESYSTEM::BASEHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::BaseHandle Constructor (protected)
//
// Arguments:
//
//	mountpoint		- Mounted file system metadata
//	alias			- Alias instance used when opening this handle
//	handle			- Handle to the host operating system object
//	flags			- Open operation flags and attributes

HostFileSystem::BaseHandle::BaseHandle(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<FileSystem::Alias>& alias, 
	HANDLE handle, int flags) : m_mountpoint(mountpoint), m_alias(alias), m_handle(handle), m_flags(flags)
{
	_ASSERTE(mountpoint);
	_ASSERTE(handle != INVALID_HANDLE_VALUE);

	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF, Exception(E_HANDLE));
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle Destructor

HostFileSystem::BaseHandle::~BaseHandle()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::DIRECTORYNODE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode Constructor
//
// Arguments:
//
//	mountpoint		- Mounted file system metadata and options
//	handle			- Query-only handle to the underlying directory object

HostFileSystem::DirectoryNode::DirectoryNode(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle) : 
	m_mountpoint(mountpoint), m_handle(handle), m_hostpath(HandleToPath(handle)) 
{
	_ASSERTE(handle != INVALID_HANDLE_VALUE);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode Destructor

HostFileSystem::DirectoryNode::~DirectoryNode()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::CreateCharacterDevice
//
// Creates a new character device node as a child of this node
//
// Arguments:
//
//	parent		- Unused; parent alias to assign to the generated child alias
//	name		- Name to assign to the new directory name
//	mode		- Mode bitmask to assign to the new directory
//	device		- Device identifier to be referenced by the new node

void HostFileSystem::DirectoryNode::CreateCharacterDevice(const FileSystem::AliasPtr&, const uapi::char_t*, uapi::mode_t, uapi::dev_t)
{
	// HostFileSystem doesn't currently support creation of special nodes
	throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::CreateDirectory
//
// Creates a new directory node as a child of this node
//
// Arguments:
//
//	parent		- Unused; parent alias to assign to the generated child alias
//	name		- Name to assign to the new directory name
//	mode		- Mode bitmask to assign to the new directory

void HostFileSystem::DirectoryNode::CreateDirectory(const FileSystem::AliasPtr&, const uapi::char_t* name, uapi::mode_t mode)
{
	tchar_t*				hostpath = nullptr;				// Completed path to the file system object

	if(name == nullptr) throw LinuxException(LINUX_EFAULT);
	if(*name == 0) throw LinuxException(LINUX_EINVAL);

	// Check that the file system is not mounted as read-only
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Convert the directory name to generic text from ANSI/UTF-8
	std::tstring namestr = std::to_tstring(name);

	// Combine the provided path with the stored path to complete the path to the target node
	HRESULT hresult = PathAllocCombine(m_hostpath, namestr.c_str(), PATHCCH_ALLOW_LONG_PATHS, &hostpath);
	if(FAILED(hresult)) throw LinuxException(LINUX_ENOMEM, Exception(hresult));

	// Set up an automatic release of the hostname string when the function exits
	onunwind freehostname([&]() -> void { LocalFree(hostpath); });

	// TODO: mode
	(mode);

	// Attempt to create the directory on the host file system
	if(!::CreateDirectory(hostpath, nullptr)) throw MapHostException(); 
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::CreateFile
//
// Creates a new regular file node as a child of this node
//
// Arguments:
//
//	parent		- Unused; Parent alias for the new object
//	name		- Name to assign to the new file alias
//	flags		- File creation flags
//	mode		- Mode bitmask to assign to the new regular file

FileSystem::HandlePtr HostFileSystem::DirectoryNode::CreateFile(const FileSystem::AliasPtr& parent, const uapi::char_t* name, int flags, uapi::mode_t mode)
{
	tchar_t*					hostpath = nullptr;				// Completed path to the file system object

	if(name == nullptr) throw LinuxException(LINUX_EFAULT);
	if(*name == 0) throw LinuxException(LINUX_EINVAL);

	// Check that the file system is not mounted as read-only
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// HostFileSystem does not support unnamed temporary files via O_TMPFILE
	if(flags & LINUX___O_TMPFILE) throw LinuxException(LINUX_EOPNOTSUPP);

	// Convert the file name to generic text from ANSI/UTF-8
	std::tstring namestr = std::to_tstring(name);

	// Combine the provided path with the stored path to complete the path to the target node
	HRESULT hresult = PathAllocCombine(m_hostpath, namestr.c_str(), PATHCCH_ALLOW_LONG_PATHS, &hostpath);
	if(FAILED(hresult)) throw LinuxException(LINUX_ENOMEM, Exception(hresult));

	// Set up an automatic release of the hostname string when the function exits
	onunwind freehostname([&]() -> void { LocalFree(hostpath); });

	// Generate the attributes for the open operation based on the provided flags
	DWORD attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS;
	if(flags & LINUX_O_SYNC) attributes |= FILE_FLAG_WRITE_THROUGH;
	if(flags & LINUX_O_DIRECT) attributes |= FILE_FLAG_NO_BUFFERING;

	// Attempt to create a new file object on the host file system with the specified attributes
	HANDLE handle = ::CreateFile(hostpath, FlagsToAccess(flags), FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW, attributes, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException();

	// Construct a new FileNode instance to represent the file system object
	std::shared_ptr<FileNode> node;
	try { node = FileNode::FromHandle(m_mountpoint, handle); }
	catch(...) { CloseHandle(handle); throw; }

	// TODO: MODE
	(mode);

	// Generate a Handle instance for the new file object
	return node->Open(Alias::Construct(name, parent, node), flags);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::CreateSymbolicLink
//
// Creates a new symbolic link as a child of this node
//
// Arguments:
//
//	parent		- Unused; parent alias for the new object
//	name		- Unused; name to assign to the new symbolic link alias
//	target		- Unused; target for the symbolic link

void HostFileSystem::DirectoryNode::CreateSymbolicLink(const FileSystem::AliasPtr&, const uapi::char_t*, const uapi::char_t*) 
{ 
	// Creation of symbolic links is not supported by HostFileSystem
	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); 
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::DemandPermission
//
// Demands read/write/execute permissions for the directory node
//
// Arguments:
//
//	mode			- Special MAY_READ, MAY_WRITE, MAY_EXECUTE flags

void HostFileSystem::DirectoryNode::DemandPermission(uapi::mode_t mode)
{
	// A mode mask of zero is F_OK, and only determines that the node exists
	if((mode & LINUX_MAY_ACCESS) == 0) return;

	// Check MAY_WRITE against a read-only file system, this produces EROFS rather than EACCES
	if((mode & LINUX_MAY_WRITE) && (m_mountpoint->Options.ReadOnly)) throw LinuxException(LINUX_EROFS);

	// TODO: NEED TO USE ACCESSCHECK() API AGAINST A SECURITY DESCRIPTOR
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::FromHandle (static)
//
// Creates a new DirectoryNode instance from a host file system object handle
//
// Arguments:
//
//	mountpoint		- Mounted file system MountPoint instance
//	handle			- Handle to the host file system object

std::shared_ptr<HostFileSystem::DirectoryNode> HostFileSystem::DirectoryNode::FromHandle(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle)
{
	return std::make_shared<DirectoryNode>(mountpoint, handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::FromPath (static)
//
// Creates a new DirectoryNode instance from a host file system path
//
// Arguments:
//
//	mountpoint		- Mounted file system MountPoint instance
//	path			- Path to the directory object on the host

std::shared_ptr<HostFileSystem::DirectoryNode> HostFileSystem::DirectoryNode::FromPath(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path)
{
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOTDIR);

	// The node type must be known in order to verify this is a directory object
	DWORD attributes = GetFileAttributes(path);
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOTDIR);
	if((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) throw LinuxException(LINUX_ENOTDIR);

	// Attempt to create a query-only handle for the underlying host file system object
	HANDLE handle = ::CreateFile(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 
		FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException();

	// Invoke the version of this method that accepts the query handle rather than the path.
	// DirectoryNode takes ownership of the handle, so close it on a construction exception
	try { return FromHandle(mountpoint, handle); }
	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::Open
//
// Creates a FileSystem::Handle instance from this node
//
// Arguments:
//
//	alias		- Alias used to resolve this node instance
//	flags		- File operation flags and attributes

FileSystem::HandlePtr HostFileSystem::DirectoryNode::Open(const AliasPtr& alias, int flags)
{
	// Duplicate the directory handle with the specified flags
	return HostFileSystem::DuplicateDirectoryHandle(m_mountpoint, alias, m_handle, flags);
}

uapi::stat HostFileSystem::DirectoryNode::getStatus(void)
{
	_RPTF0(_CRT_ASSERT, "HostFileSystem::DirectoryNode::ReadStatus -- not implemented yet");
	return uapi::stat();
}

void HostFileSystem::DirectoryNode::RemoveNode(const uapi::char_t* name)
{
	(name);
	throw Exception(E_NOTIMPL);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::Resolve
//
// Resolves an Alias instance for a path relative to this node
//
// Arguments:
//
//	root			- Unused; root lookups never happen
//	current			- Current alias instance in the path resolution
//	path			- Relative path from this node to be resolved
//	flags			- Path resolution flags
//	symlinks		- Unused; symlinks are processed by the host operating system

FileSystem::AliasPtr HostFileSystem::DirectoryNode::Resolve(const AliasPtr&, const AliasPtr& current, const uapi::char_t* path, int flags, int*)
{
	tchar_t*					hostpath = nullptr;				// Completed path to the file system object

	if(path == nullptr) throw LinuxException(LINUX_ENOENT);		// NULL path strings are disallowed

	(flags); // TODO - WORK IN PROGRESS -- NEED TO IMPLEMENT THESE -- SEE TEMPFILESYSTEM

	// If a blank path has been specified, the resolution process ends at this node
	if(path[0] == '\0') return current;

	std::tstring pathstr = std::to_tstring(path);				// Convert relative path from ANSI/UTF-8

	// Combine the provided path with the stored path to complete the path to the target node
	HRESULT hresult = PathAllocCombine(m_hostpath, pathstr.c_str(), PATHCCH_ALLOW_LONG_PATHS, &hostpath);
	if(FAILED(hresult)) throw LinuxException(LINUX_ENOMEM, Exception(hresult));

	// Set up an automatic release of the hostname string when the function exits
	onunwind freehostname([&]() -> void { LocalFree(hostpath); });

	// Extract the file name from the path and convert it to ANSI/UTF-8 for the alias instance
	std::string aliasname = std::to_string(PathFindFileName(hostpath));

	// Retrieve the basic attributes for the node to determine if it's a file or a directory
	DWORD attributes = GetFileAttributes(hostpath);
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOENT);

	// Generate either a directory or file alias based on what the underlying object type happens to be
	if((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) 
		return Alias::Construct(aliasname.c_str(), DirectoryNode::FromPath(m_mountpoint, hostpath));
	else return Alias::Construct(aliasname.c_str(), FileNode::FromPath(m_mountpoint, hostpath));
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::getIndex
//
// Gets the file index for this node from the operating system

//uint64_t HostFileSystem::DirectoryNode::getIndex(void)
//{
//	return HandleToIndex(m_handle);
//}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::getType
//
// Gets the node type for this object

FileSystem::NodeType HostFileSystem::DirectoryNode::getType(void)
{
	return FileSystem::NodeType::Directory;
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::DIRECTORYHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Duplicate
//
// Duplicates the Handle object, potentially with new flags
//
// Arguments:
//
//	flags		- Flags applicable to the new handle

FileSystem::HandlePtr HostFileSystem::DirectoryHandle::Duplicate(int flags)
{
	// Duplicate the directory handle with the specified flags
	return HostFileSystem::DuplicateDirectoryHandle(m_mountpoint, m_alias, m_handle, flags);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Pointer to the output buffer
//	count		- Number of bytes to read from the file, also minimum size of buffer

uapi::size_t HostFileSystem::DirectoryHandle::Read(void*, uapi::size_t)
{
	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
}

uapi::size_t HostFileSystem::DirectoryHandle::ReadAt(uapi::loff_t, void*, uapi::size_t)
{
	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Seek
//
// Sets the file pointer for this handle
//
// Arguments:
//
//	offset		- Offset into the file to set the pointer, based on whence
//	whence		- Starting position within the file to apply offset

uapi::loff_t HostFileSystem::DirectoryHandle::Seek(uapi::loff_t, int)
{
	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void HostFileSystem::DirectoryHandle::Sync(void)
{
	// DO NOTHING
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void HostFileSystem::DirectoryHandle::SyncData(void)
{
	// DO NOTHING
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer			- Unused, input buffer from which to write the data
//	count			- Unused, number of bytes to write from the input buffer

uapi::size_t HostFileSystem::DirectoryHandle::Write(const void*, uapi::size_t)
{
	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
}

uapi::size_t HostFileSystem::DirectoryHandle::WriteAt(uapi::loff_t, const void*, uapi::size_t)
{
	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::EXECUTEHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::ExecuteHandle::Duplicate
//
// Duplicates the Handle object, potentially with new flags
//
// Arguments:
//
//	flags		- Flags applicable to the new handle

FileSystem::HandlePtr HostFileSystem::ExecuteHandle::Duplicate(int flags)
{
	// Duplicate the file handle with the specified flags
	return HostFileSystem::DuplicateFileHandle(m_mountpoint, m_alias, m_handle, flags);
}

//-----------------------------------------------------------------------------
// HostFileSystem::ExecuteHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Pointer to the output buffer
//	count		- Number of bytes to read from the file, also minimum size of buffer

uapi::size_t HostFileSystem::ExecuteHandle::Read(void* buffer, uapi::size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// ReadFile() can only read up to MAXDWORD bytes from the underlying file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL, Exception(E_BOUNDS));

	// Attempt to read the specified number of bytes from the file into the buffer
	DWORD read;
	if(!ReadFile(m_handle, buffer, static_cast<DWORD>(count), &read, nullptr)) throw MapHostException();

	return static_cast<uapi::size_t>(read);
}

// just hacked in for now
uapi::size_t HostFileSystem::ExecuteHandle::ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count)
{
	LARGE_INTEGER			oldposition;
	LARGE_INTEGER			newposition;

	oldposition.QuadPart = 0;
	newposition.QuadPart = offset;

	// Get the current position
	if(!SetFilePointerEx(m_handle, oldposition, &oldposition, FILE_CURRENT)) throw MapHostException();

	// Set the new position
	if(!SetFilePointerEx(m_handle, newposition, nullptr, FILE_BEGIN)) throw MapHostException();

	size_t result;
	
	try { result = Read(buffer, count); }
	catch(...) { SetFilePointerEx(m_handle, oldposition, nullptr, FILE_BEGIN); throw; }

	SetFilePointerEx(m_handle, oldposition, nullptr, FILE_BEGIN);
	return result;	
}

//-----------------------------------------------------------------------------
// HostFileSystem::ExecuteHandle::Seek
//
// Sets the file pointer for this handle
//
// Arguments:
//
//	offset		- Offset into the file to set the pointer, based on whence
//	whence		- Starting position within the file to apply offset

uapi::loff_t HostFileSystem::ExecuteHandle::Seek(uapi::loff_t offset, int whence)
{
	LARGE_INTEGER			position;			// Distance to be moved

	// This is a simple operation on the host, provided the values for whence mean the same
	// thing, which is checked as an invariant at the top of this file
	position.QuadPart = offset;
	if(!SetFilePointerEx(m_handle, position, &position, whence)) throw MapHostException();

	return position.QuadPart;
}

//-----------------------------------------------------------------------------
// HostFileSystem::ExecuteHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void HostFileSystem::ExecuteHandle::Sync(void)
{
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::ExecuteHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void HostFileSystem::ExecuteHandle::SyncData(void)
{
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::ExecuteHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer			- Unused, input buffer from which to write the data
//	count			- Unused, number of bytes to write from the input buffer

uapi::size_t HostFileSystem::ExecuteHandle::Write(const void*, uapi::size_t)
{
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

uapi::size_t HostFileSystem::ExecuteHandle::WriteAt(uapi::loff_t, const void*, uapi::size_t)
{
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::FILEHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle Constructor
//
// Arguments:
//
//	mountpoint		- Mounted file system metadata
//	alias			- Alias instance used when opening this handle
//	handle			- Handle to the host operating system object
//	flags			- Open operation flags and attributes

HostFileSystem::FileHandle::FileHandle(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<FileSystem::Alias>& alias, 
	HANDLE handle, int flags) : BaseHandle(mountpoint, alias, handle, flags)
{
	// O_DIRECT operations require a specific alignment when reading/writing to
	// the file, get that information from the operating system
	if(flags & LINUX_O_DIRECT) {

		FILE_STORAGE_INFO storageinfo;
		if(!GetFileInformationByHandleEx(handle, FileStorageInfo, &storageinfo, sizeof(storageinfo))) throw MapHostException();
		m_alignment = storageinfo.FileSystemEffectivePhysicalBytesPerSectorForAtomicity;
	}
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Duplicate
//
// Duplicates the Handle object, potentially with new flags
//
// Arguments:
//
//	flags		- Flags applicable to the new handle

FileSystem::HandlePtr HostFileSystem::FileHandle::Duplicate(int flags)
{
	// Duplicate the file handle with the specified flags
	return HostFileSystem::DuplicateFileHandle(m_mountpoint, m_alias, m_handle, flags);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Pointer to the output buffer
//	count		- Number of bytes to read from the file, also minimum size of buffer

uapi::size_t HostFileSystem::FileHandle::Read(void* buffer, uapi::size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// ReadFile() can only read up to MAXDWORD bytes from the underlying file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL, Exception(E_BOUNDS));

	// If the handle was opened with O_DIRECT, the buffer, count and current offset must be aligned
	if(m_flags & LINUX_O_DIRECT) VerifyDirectAlignment(buffer, count);

	// Attempt to read the specified number of bytes from the file into the buffer
	DWORD read;
	if(!ReadFile(m_handle, buffer, static_cast<DWORD>(count), &read, nullptr)) throw MapHostException();

	return static_cast<uapi::size_t>(read);
}

// just hacked in for now
uapi::size_t HostFileSystem::FileHandle::ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count)
{
	LARGE_INTEGER			oldposition;
	LARGE_INTEGER			newposition;

	oldposition.QuadPart = 0;
	newposition.QuadPart = offset;

	// Get the current position
	if(!SetFilePointerEx(m_handle, oldposition, &oldposition, FILE_CURRENT)) throw MapHostException();

	// Set the new position
	if(!SetFilePointerEx(m_handle, newposition, nullptr, FILE_BEGIN)) throw MapHostException();

	size_t result;
	
	try { result = Read(buffer, count); }
	catch(...) { SetFilePointerEx(m_handle, oldposition, nullptr, FILE_BEGIN); throw; }

	SetFilePointerEx(m_handle, oldposition, nullptr, FILE_BEGIN);
	return result;	
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Seek
//
// Sets the file pointer for this handle
//
// Arguments:
//
//	offset		- Offset into the file to set the pointer, based on whence
//	whence		- Starting position within the file to apply offset

uapi::loff_t HostFileSystem::FileHandle::Seek(uapi::loff_t offset, int whence)
{
	LARGE_INTEGER			position;			// Distance to be moved

	// This is a simple operation on the host, provided the values for whence mean the same
	// thing, which is checked as an invariant at the top of this file
	position.QuadPart = offset;
	if(!SetFilePointerEx(m_handle, position, &position, whence)) throw MapHostException();

	return position.QuadPart;
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void HostFileSystem::FileHandle::Sync(void)
{
	// The closest equivalent for this operation is FlushFileBuffers()
	if(!FlushFileBuffers(m_handle)) throw MapHostException();
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void HostFileSystem::FileHandle::SyncData(void)
{
	// The closest equivalent for this operation is FlushFileBuffers()
	if(!FlushFileBuffers(m_handle)) throw MapHostException();
}

//-----------------------------------------------------------------------------
//  HostFileSystem::FileHandle::VerifyDirectAlignment (protected)
//
// Used with O_DIRECT, this validates that the memory buffer pointer as well as
// the handle file pointer have the proper alignment
//
// Arguments:
//
//	buffer		- Pointer to the input/output buffer
//	count		- Size of the input/output buffer, in bytes

void  HostFileSystem::FileHandle::VerifyDirectAlignment(const void* buffer, uapi::size_t count)
{
	LARGE_INTEGER pointer = { 0, 0 };

	if(!buffer) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// Retrieve the current file pointer offset so that it can be validated with the other criteria
	if(!SetFilePointerEx(m_handle, pointer, &pointer, FILE_CURRENT)) throw MapHostException();

	// The memory buffer, the current file offset and the number of bytes to operate on must all be
	// a multiple of the host file system's alignment requirement
	if((uintptr_t(buffer) % m_alignment) || (pointer.QuadPart % m_alignment) || (count % m_alignment)) 
		throw LinuxException(LINUX_EINVAL, Win32Exception(ERROR_OFFSET_ALIGNMENT_VIOLATION));
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer			- Unused, input buffer from which to write the data
//	count			- Unused, number of bytes to write from the input buffer

uapi::size_t HostFileSystem::FileHandle::Write(const void* buffer, uapi::size_t count)
{
	DWORD				written;			// Bytes written to the file

	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// WriteFile() can only write up to MAXDWORD bytes to the underlying file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL, Exception(E_BOUNDS));

	// There is no way to make O_APPEND atomic with HostFileSystem, just set it and hope for the best
	if((m_flags & LINUX_O_APPEND) && (SetFilePointer(m_handle, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER)) throw MapHostException();

	// If the handle was opened with O_DIRECT, the buffer, count and current offset must be aligned
	if(m_flags & LINUX_O_DIRECT) VerifyDirectAlignment(buffer, count);

	// Attempt to write the specified number of bytes from the buffer to the file
	if(!WriteFile(m_handle, buffer, static_cast<DWORD>(count), &written, nullptr)) throw MapHostException();

	return static_cast<uapi::size_t>(written);
}

// just hacked in for now
uapi::size_t HostFileSystem::FileHandle::WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count)
{
	// can't do this in append mode
	if(m_flags & LINUX_O_APPEND) throw LinuxException(LINUX_EINVAL);

	LARGE_INTEGER			oldposition;
	LARGE_INTEGER			newposition;

	oldposition.QuadPart = 0;
	newposition.QuadPart = offset;

	// Get the current position
	if(!SetFilePointerEx(m_handle, oldposition, &oldposition, FILE_CURRENT)) throw MapHostException();

	// Set the new position
	if(!SetFilePointerEx(m_handle, newposition, nullptr, FILE_BEGIN)) throw MapHostException();

	size_t result;
	
	try { result = Write(buffer, count); }
	catch(...) { SetFilePointerEx(m_handle, oldposition, nullptr, FILE_BEGIN); throw; }

	SetFilePointerEx(m_handle, oldposition, nullptr, FILE_BEGIN);
	return result;	
}
//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::FILENODE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode Constructor
//
// Arguments:
//
//	mountpoint		- Mounted file system metadata and options
//	handle			- Query-only handle to the underlying file object

HostFileSystem::FileNode::FileNode(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle) : m_mountpoint(mountpoint), m_handle(handle)
{
	_ASSERTE(handle != INVALID_HANDLE_VALUE);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode Destructor

HostFileSystem::FileNode::~FileNode()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::DemandPermission
//
// Demands read/write/execute permissions for the file node
//
// Arguments:
//
//	mode			- Special MAY_READ, MAY_WRITE, MAY_EXECUTE flags

void HostFileSystem::FileNode::DemandPermission(uapi::mode_t mode)
{
	// A mode mask of zero is F_OK, and only determines that the node exists
	if((mode & LINUX_MAY_ACCESS) == 0) return;

	// Check MAY_WRITE against a read-only file system, this produces EROFS rather than EACCES
	if((mode & LINUX_MAY_WRITE) && (m_mountpoint->Options.ReadOnly)) throw LinuxException(LINUX_EROFS);

	// --> ?? GetSecurityInfo(m_handle, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION, nullptr, nullptr, nullptr, nullptr, *secdesc);
	// TODO: NEED TO USE ACCESSCHECK() API AGAINST A SECURITY DESCRIPTOR
	// GetFileSecurity then accesscheck() here
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::FromHandle (static)
//
// Creates a new FileNode instance from a host file system object handle
//
// Arguments:
//
//	mountpoint		- Mounted file system MountPoint instance
//	handle			- Handle to the host file system object

std::shared_ptr<HostFileSystem::FileNode> HostFileSystem::FileNode::FromHandle(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle)
{
	return std::make_shared<FileNode>(mountpoint, handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::FromPath (static)
//
// Creates a new FileNode instance from a host file system path
//
// Arguments:
//
//	mountpoint		- Mounted file system MountPoint instance
//	path			- Path to the file object on the host

std::shared_ptr<HostFileSystem::FileNode> HostFileSystem::FileNode::FromPath(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path)
{
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// The node type must be known in order to verify this is a file object
	DWORD attributes = GetFileAttributes(path);
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOENT);
	if((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) throw LinuxException(LINUX_EISDIR);

	// Attempt to create a query-only handle for the underlying host file system object
	HANDLE handle = ::CreateFile(path, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_POSIX_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException();

	// Invoke the version of this method that accepts the query handle rather than the path.
	// FileNode takes ownership of the handle, so close it on a construction exception
	try { return FromHandle(mountpoint, handle); }
	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::Open
//
// Creates a FileSystem::Handle instance from this node
//
// Arguments:
//
//	alias		- Alias instance used to resolve this node
//	flags		- File operation flags and attributes

FileSystem::HandlePtr HostFileSystem::FileNode::Open(const AliasPtr& alias, int flags)
{
	return HostFileSystem::DuplicateFileHandle(m_mountpoint, alias, m_handle, flags);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::OpenExec
//
// Creates an execute-only FileSystem::Handle instance for this node
//
// Arguments:
//
//	alias		- Aliased used to resolve this node instance

FileSystem::HandlePtr HostFileSystem::FileNode::OpenExec(const AliasPtr& alias)
{
	// If the file system was mounted with noexec, this file cannot be executed
	if(m_mountpoint->Options.NoExecute) throw LinuxException(LINUX_EACCES);

	// Re-open the underlying handle with EXECUTE and READ access and only allow shared reads
	HANDLE handle = ReOpenFile(m_handle, FILE_GENERIC_EXECUTE | FILE_GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_FLAG_POSIX_SEMANTICS);
	if(handle == INVALID_HANDLE_VALUE) throw MapHostException();

	// Construct and return an ExecuteHandle instance; it takes ownership of the handle
	try { return std::make_shared<ExecuteHandle>(m_mountpoint, alias, handle, 0); }
	catch(...) { CloseHandle(handle); throw; }
}

uapi::stat HostFileSystem::FileNode::getStatus(void)
{
	_RPTF0(_CRT_ASSERT, "HostFileSystem::FileNode::ReadStatus -- not implemented yet");
	return uapi::stat();
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::Resolve
//
// Resolves an Alias instance for a path relative to this node
//
// Arguments:
//
//	root			- Unused
//	current			- Current alias instance in the path resolution process
//	path			- Relative path from this node to be resolved, must be blank
//	flags			- Path resolution flags
//	symlinks		- Unused

FileSystem::AliasPtr HostFileSystem::FileNode::Resolve(const AliasPtr&, const AliasPtr& current, const uapi::char_t* path, int flags, int*)
{
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// If the path operation required termination in a directory, it cannot end here
	if((flags & LINUX_O_DIRECTORY) == LINUX_O_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);

	// File nodes can only be resolved to themselves, they have no children
	if(path[0] != '\0') throw LinuxException(LINUX_ENOTDIR);
	return current;
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::getIndex
//
// Gets the file index for this node from the operating system

//uint64_t HostFileSystem::FileNode::getIndex(void)
//{
//	return HandleToIndex(m_handle);
//}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::getType
//
// Gets the node type for this object

FileSystem::NodeType HostFileSystem::FileNode::getType(void)
{
	return FileSystem::NodeType::File;
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::PATHHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::PathHandle::Duplicate
//
// Duplicates the Handle object, potentially with new flags
//
// Arguments:
//
//	flags		- Flags applicable to the new handle

FileSystem::HandlePtr HostFileSystem::PathHandle::Duplicate(int flags)
{
	// PathHandle instances may refer to either a file or a directory node
	BY_HANDLE_FILE_INFORMATION info;
	if(!GetFileInformationByHandle(m_handle, &info)) throw MapHostException();

	// Duplicate to either a directory or file handle based on the underlying object type
	if((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) 
		return HostFileSystem::DuplicateDirectoryHandle(m_mountpoint, m_alias, m_handle, flags);
	else return HostFileSystem::DuplicateFileHandle(m_mountpoint, m_alias, m_handle, flags);
}

//-----------------------------------------------------------------------------
// HostFileSystem::PathHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Pointer to the output buffer
//	count		- Number of bytes to read from the file, also minimum size of buffer

uapi::size_t HostFileSystem::PathHandle::Read(void*, uapi::size_t)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

uapi::size_t HostFileSystem::PathHandle::ReadAt(uapi::loff_t, void*, uapi::size_t)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::PathHandle::Seek
//
// Sets the file pointer for this handle
//
// Arguments:
//
//	offset		- Offset into the file to set the pointer, based on whence
//	whence		- Starting position within the file to apply offset

uapi::loff_t HostFileSystem::PathHandle::Seek(uapi::loff_t, int)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::PathHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void HostFileSystem::PathHandle::Sync(void)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::PathHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void HostFileSystem::PathHandle::SyncData(void)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// HostFileSystem::PathHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer			- Unused, input buffer from which to write the data
//	count			- Unused, number of bytes to write from the input buffer

uapi::size_t HostFileSystem::PathHandle::Write(const void*, uapi::size_t)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

uapi::size_t HostFileSystem::PathHandle::WriteAt(uapi::loff_t, const void*, uapi::size_t)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

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
//	datalen		- Length of available custom mounting options

HostFileSystem::MountPoint::MountPoint(HANDLE handle, uint32_t flags, const void* data, size_t datalen) : 
	m_handle(handle), m_options(flags, data, datalen), m_hostpath(HandleToPath(handle))
{
	_ASSERTE(handle != INVALID_HANDLE_VALUE);
}

//-----------------------------------------------------------------------------
// HostFileSystem::MountPoint Destructor

HostFileSystem::MountPoint::~MountPoint()
{
	// Close the operating system handle that references the mount point
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
