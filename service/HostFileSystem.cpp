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
// HostFileSystem Constructor
//
// Arguments:
//
//	mountpoint	- Reference to the mountpoint instance for this file system
//	root		- Pointer to the root node for the file system

HostFileSystem::HostFileSystem(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Node>& root) 
	: m_mountpoint(mountpoint), m_root(root)
{
	_ASSERTE(mountpoint);
	_ASSERTE(root);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount (static)
//
// Mounts the host file system by opening the specified directory
//
// Arguments:
//
//	source		- Path to the root file system node on the host
//	flags		- Mounting flags and attributes
//	data		- Additional file-system specific mounting options

FileSystemPtr HostFileSystem::Mount(const tchar_t* source, uint32_t flags, void* data)
{
	std::shared_ptr<MountPoint>			mountpoint;			// MountPoint instance

	//
	// TODO: how will this behave if a symbolic link to a directory is provided?
	//

	// Determine the type of node that the path represents; must be a directory for mounting
	FileSystem::NodeType type = NodeTypeFromPath(source);
	if(type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// Attempt to open a query-only handle against the file system object
	HANDLE handle = CreateFile(source, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	// Attempt to create the MountPoint instance for this file system against the handle
	try { mountpoint = std::make_shared<MountPoint>(handle, flags, data); }
	catch(...) { CloseHandle(handle); throw; }

	// Construct the HostFileSystem instance with a root Node instance that references the base path
	return std::make_shared<HostFileSystem>(mountpoint, NodeFromPath(mountpoint, mountpoint->BasePath));
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeFromPath (private, static)
//
// Creates a HostFileSystem::Node instance based on a host path
//
// Arguments:
//
//	mountpoint	- Reference to the mountpoint instance for this file system
//	path		- Host operating system path to construct the node against

std::shared_ptr<HostFileSystem::Node> HostFileSystem::NodeFromPath(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path)
{
	_ASSERTE(path);
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Convert the path string into a vector<> that can be moved into the Node instance
	std::vector<tchar_t> pathvec(_tcslen(path) + 1);
	memcpy(pathvec.data(), path, pathvec.size() * sizeof(tchar_t));

	return NodeFromPath(mountpoint, std::move(pathvec));
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeFromPath (private, static)
//
// Creates a HostFileSystem::Node instance based on a host path
//
// Arguments:
//
//	mountpoint	- Reference to the mountpoint instance for this file system
//	path		- Host operating system path to construct the node against

std::shared_ptr<HostFileSystem::Node> HostFileSystem::NodeFromPath(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path)
{
	// All nodes in the host file system are detached, no operating system handle is opened
	return std::make_shared<Node>(mountpoint, std::move(path), NodeTypeFromPath(path.data()));
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeTypeFromPath (private, static)
//
// Generates the FileSystem::NodeType that corresponds to the host attribute
// flags for a given path.  Will throw if the path does not exist
//
// Arguments:
//
//	path		- Host operating system path to the node to query
//	attributes	- Optional variable to receive the windows attribute flags

FileSystem::NodeType HostFileSystem::NodeTypeFromPath(const tchar_t* path, DWORD* attributes)
{
	_ASSERTE(path);

	if(attributes) *attributes = 0;						// Initialize [out] argument

	// Query the basic attributes about the specified path
	DWORD attrs = GetFileAttributes(path);
	if(attrs == INVALID_FILE_ATTRIBUTES) {

		// If the path does not exist, FILE_NOT_FOUND or PATH_NOT_FOUND would be set, anything
		// else raise as an ENOENT exception to the caller
		DWORD winerror = GetLastError();
		if((winerror == ERROR_FILE_NOT_FOUND) || (winerror == ERROR_PATH_NOT_FOUND)) return FileSystem::NodeType::Empty;
		// todo: need LinuxExceptionFromWin32() that maps things like error 5 to EACCES
		// ERROR_BAD_NETPATH? this API doesn't work on shares like \\SERVER\SHARE, which should be OK, BUT that would
		// be the mountpoint, so ... dunno
		else throw LinuxException(LINUX_ENOENT, Win32Exception(winerror));
	}

	// The caller may need the underlying windows file attribute mask
	if(attributes) *attributes = attrs;

	// Check for REPARSE_POINT first as it will be combined with other flags indicating if this is a directory or a file
	if((attrs & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) return FileSystem::NodeType::SymbolicLink;
	else if((attrs & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) return FileSystem::NodeType::Directory;
	else return FileSystem::NodeType::File;
}

//
// HOSTFILESYSTEM::HANDLE
//

HostFileSystem::Handle::Handle(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Node>& node, HANDLE handle, int flags)
	: m_alignment(1), m_mountpoint(mountpoint), m_node(node), m_handle(handle), m_flags(flags)
{
	FILE_STORAGE_INFO			storageinfo;			// Information about underyling storage

	// Handles opened with O_DIRECT require the data to be aligned to the physical sector size
	if(flags & LINUX_O_DIRECT) {

		if(!GetFileInformationByHandleEx(handle, FileStorageInfo, &storageinfo, sizeof(storageinfo))) 
			throw LinuxException(LINUX_EIO, Win32Exception());

		// Reset the required O_DIRECT alignment based on the file system physical sector size
		m_alignment = storageinfo.FileSystemEffectivePhysicalBytesPerSectorForAtomicity;
	}
}

HostFileSystem::Handle::~Handle()
{
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

	
uapi::size_t HostFileSystem::Handle::Read(void* buffer, uapi::size_t count)
{
	DWORD				read;			// Bytes read from the file

	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	_ASSERTE(m_node);

	if(m_handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF);

	// Host file system does not support reading from directories or symbolic links
	if(m_node->Type == FileSystem::NodeType::Directory) throw LinuxException(LINUX_EISDIR);
	else if(m_node->Type != FileSystem::NodeType::File) throw LinuxException(LINUX_EINVAL);

	// If the handle was opened with O_DIRECT, the buffer, count and current offset must be aligned
	if(m_flags & LINUX_O_DIRECT) ValidateAlignment(buffer, count);

	// Attempt to read the specified number of bytes from the file into the buffer
	if(!ReadFile(m_handle, buffer, count, &read, nullptr)) throw LinuxException(LINUX_EIO, Win32Exception());

	return static_cast<uapi::size_t>(read);
}

//-----------------------------------------------------------------------------


void HostFileSystem::Handle::Sync(void)
{
	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);

	// The closest equivalent of fsync/fdatasync on Windows is FlushFileBuffers()
	if(!FlushFileBuffers(m_handle)) throw LinuxException(LINUX_EIO, Win32Exception());
}

uapi::size_t HostFileSystem::Handle::Write(const void* buffer, uapi::size_t count)
{
	DWORD				written;		// Bytes written to the file

	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	_ASSERTE(m_node);

	if(m_handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF);

	// Host file system does not support writing to directories or symbolic links
	if(m_node->Type == FileSystem::NodeType::Directory) throw LinuxException(LINUX_EISDIR);
	else if(m_node->Type != FileSystem::NodeType::File) throw LinuxException(LINUX_EINVAL);

	// There is no way to make O_APPEND atomic on the host file system, just move it and hope for the best
	if((m_flags & LINUX_O_APPEND) && (SetFilePointer(m_handle, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER))
		throw LinuxException(LINUX_EIO, Win32Exception());

	// If the handle was opened with O_DIRECT, the buffer, count and current offset must be aligned
	if(m_flags & LINUX_O_DIRECT) ValidateAlignment(buffer, count);

	// Attempt to write the specified number of bytes from the buffer into the file
	if(!WriteFile(m_handle, buffer, count, &written, nullptr)) throw LinuxException(LINUX_EIO, Win32Exception());

	return static_cast<uapi::size_t>(written);
}

void HostFileSystem::Handle::ValidateAlignment(const void* buffer, uapi::size_t count)
{
	LARGE_INTEGER pointer = { 0, 0 };

	if(!buffer) throw LinuxException(LINUX_EINVAL, Exception(E_POINTER));

	// Retrieve the current file pointer offset so that it can be validated with the other criteria
	if(!SetFilePointerEx(m_handle, pointer, &pointer, FILE_CURRENT)) throw LinuxException(LINUX_EIO, Win32Exception());

	// The memory buffer, the current file offset and the number of bytes to operate on must all be
	// a multiple of the host file system's alignment requirement
	if((uintptr_t(buffer) % m_alignment) || (pointer.QuadPart % m_alignment) || (count % m_alignment)) 
		throw LinuxException(LINUX_EINVAL, Win32Exception(ERROR_OFFSET_ALIGNMENT_VIOLATION));
}

//
// HOSTFILESYSTEM::MOUNTPOINT
//

//-----------------------------------------------------------------------------
// HostFileSystem::MountPoint Constructor

HostFileSystem::MountPoint::MountPoint(HANDLE handle, uint32_t flags, const void* data) : m_handle(handle), m_options(flags, data)
{
	_ASSERTE(handle != INVALID_HANDLE_VALUE);

	// Determine the amount of space that needs to be allocated for the canonicalized path name string; when 
	// providing NULL for the output, this will include the count for the NULL terminator
	uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

	// Retrieve the canonicalized path to the directory object based on the handle; this will serve as the base
	m_path.resize(pathlen);
	pathlen = GetFinalPathNameByHandle(handle, m_path.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());
}

//-----------------------------------------------------------------------------
// HostFileSystem::MountPoint Destructor

HostFileSystem::MountPoint::~MountPoint()
{
	// Close the operating system handle that references the mount point
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::MountPoint::ValidateHandle
//
// Validates that a newly opened file system handle meets the criteria set
// for this host mount point
//
// Arguments:
//
//	handle		- Operating system handle to validate

void HostFileSystem::MountPoint::ValidateHandle(HANDLE handle)
{
	// If path verification is active, get the canonicalized name associated with this
	// handle and ensure that it is a child of the mounted root directory; this prevents
	// access outside of the mountpoint by symbolic links or ".." parent path components
	if(m_verifypath) {

		// Determine the amount of space that needs to be allocated for the directory path name string; when 
		// providing NULL for the output, this will include the count for the NULL terminator
		uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

		// If the path is at least as long as the original mountpoint name, it cannot be a child
		if(pathlen < m_path.size()) throw LinuxException(LINUX_EPERM);

		// Retrieve the path to the directory object based on the handle
		std::vector<tchar_t> path(pathlen);
		pathlen = GetFinalPathNameByHandle(handle, path.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

		// Verify that the canonicalized path starts with the mount point, don't ignore case
		if(_tcsncmp(m_path.data(), path.data(), m_path.size() - 1) != 0) throw LinuxException(LINUX_EPERM);
	}
}

//
// HOSTFILESYSTEM::NODE
//

//-----------------------------------------------------------------------------
// HostFileSystem::Node Constructor
//
// Arguments:
//
//	mountpoint	- Reference to the file system mountpoint instance
//	path		- vector<> containing the operating system path
//	type		- Type of node being constructed
//	handle		- Open operating system handle for the node (query access)

HostFileSystem::Node::Node(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path, FileSystem::NodeType type)
	: m_mountpoint(mountpoint), m_path(std::move(path)), m_type(type)
{
	// Get a pointer to the path leaf name to satisfy the FileSystem::Alias interface
	m_name = PathFindFileName(m_path.data());
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::AppendToPath (private)
//
// Appends additional path information to this node's underlying path
//
// Arguments:
//
//	more		- Additional path information to be appended

std::vector<tchar_t> HostFileSystem::Node::AppendToPath(const tchar_t* more)
{
	// A null or zero-length string will result in a copy of the node path
	if((more == nullptr) || (*more == 0)) return std::vector<tchar_t>(m_path);
	
	// Copy the path to this node and add space for a possible "\\?\" prefix in the
	// event that the combined path will require it (if it now exceeds MAX_PATH)
	std::vector<tchar_t> pathvec(m_path);
	pathvec.resize(pathvec.size() + _tcslen(more) + 5);

	// Append the provided path to this node's path
	HRESULT hresult = PathCchAppendEx(pathvec.data(), pathvec.size(), more, PATHCCH_ALLOW_LONG_PATHS);
	if(FAILED(hresult)) throw LinuxException(LINUX_ENAMETOOLONG, Exception(hresult));
	
	// Trim the excess from the end of the path string before returning it
	pathvec.resize(_tcslen(pathvec.data()) + 1);
	return pathvec;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::CreateDirectory (private)
//
// Creates a new directory node as a child of this node
//
// Arguments:
//
//	name		- Name to assign to the new directory name

void HostFileSystem::Node::CreateDirectory(const tchar_t* name)
{
	// Cannot create a directory with a null or zero-length name
	if((name == nullptr) || (*name == 0)) throw LinuxException(LINUX_EINVAL);

	// Check that the file system is not mounted as read-only
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Combine the name with the base path for this node and attempt to create it
	std::vector<tchar_t> pathvec = AppendToPath(name);
	if(!::CreateDirectory(pathvec.data(), nullptr)) {

		DWORD result = GetLastError();			// Get Windows error code

		// Try to map the Windows error into something more appropriate
		if(result == ERROR_ALREADY_EXISTS) throw LinuxException(LINUX_EEXIST, Win32Exception(result));
		else if(result == ERROR_PATH_NOT_FOUND) throw LinuxException(LINUX_ENOENT, Win32Exception(result));
		else throw LinuxException(LINUX_EINVAL, Win32Exception(result));
	}
}

FileSystem::AliasPtr HostFileSystem::Node::CreateFile(const tchar_t* name)
{
	// Cannot create a directory with a null or zero-length name
	if((name == nullptr) || (*name == 0)) throw LinuxException(LINUX_EINVAL);

	// Check that the file system is not mounted as read-only
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Combine the name with the base path for this node and attempt to create it
	std::vector<tchar_t> pathvec = AppendToPath(name);

	// need something like Node::CreateFromHandle, which returns not only a node
	// or alias pointer, but also a Handle pointer.  Then I can call CreateFile
	// here and not need to reopen the stupid thing

	// todo
	return nullptr;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::FlagsToAccess (static, private)
//
// Converts linux fnctl flags to Windows access mode flags for CreateFile
//
// Arguments:
//
//	flags		- Linux fnctl flags to be converted

inline DWORD HostFileSystem::Node::FlagsToAccess(int flags)
{
	switch(flags & LINUX_O_ACCMODE) {

		// O_RDONLY --> GENERIC_READ
		case LINUX_O_RDONLY: return GENERIC_READ;

		// O_WRONLY --> GENERIC_WRITE
		case LINUX_O_WRONLY: return GENERIC_WRITE;

		// O_RDWR --> GENERIC_READ | GENERIC_WRITE;
		case LINUX_O_RDWR: return GENERIC_READ | GENERIC_WRITE;
	}

	// Possible exception if both O_WRONLY (1) and O_RDWR (2) are set
	throw LinuxException(LINUX_EINVAL);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::OpenHandle
//
// Creates a FileSystem::Handle instance for this node on the specified alias
//
// Arguments:
//
//	alias		- Reference to the Alias instance that was resolved
//	flags		- Open flags from the caller

FileSystem::HandlePtr HostFileSystem::Node::OpenHandle(const FileSystem::AliasPtr& alias, int flags)
{
	UNREFERENCED_PARAMETER(alias);

	// The host file system cannot support unnamed temporary files via O_TMPFILE
	if(flags & LINUX___O_TMPFILE) throw LinuxException(LINUX_EINVAL);

	// Convert the file control flags into the access flags and check for read-only file system
	DWORD access = FlagsToAccess(flags);
	if((m_mountpoint->Options.ReadOnly) && (access & GENERIC_WRITE)) throw LinuxException(LINUX_EROFS);

	// Generate the disposition flags for the open operation
	DWORD disposition = 0;

	// Generate the attributes for the open operation based on the node type and provided flags
	DWORD attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS;
	if(m_type == FileSystem::NodeType::Directory) attributes |= FILE_FLAG_BACKUP_SEMANTICS;
	else if(m_type == FileSystem::NodeType::SymbolicLink) attributes |= FILE_FLAG_OPEN_REPARSE_POINT;
	if(flags & LINUX_O_SYNC) attributes |= FILE_FLAG_WRITE_THROUGH;
	if(flags & LINUX_O_DIRECT) attributes |= FILE_FLAG_NO_BUFFERING;

	// Open a handle ... todo words
	HANDLE handle = ::CreateFile(m_path.data(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, disposition, attributes, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EINVAL, Win32Exception());

	try {

		// This goes in disposition now
		//// By using ReOpenFile() rather than CreateFile() to generate the handle, there is no opportunity
		//// to specify TRUNCATE_EXISTING in the disposition flags; do that now
		//if((access & GENERIC_WRITE) && (flags & LINUX_O_TRUNC))
		//	if(!SetEndOfFile(handle)) throw LinuxException(LINUX_EIO, Win32Exception());

		// Generate a new Handle instance around the new object handle and flags
		return std::make_shared<Handle>(m_mountpoint, shared_from_this(), handle, flags);
	}
	
	catch(...) { CloseHandle(handle); throw; }
}
		
//-----------------------------------------------------------------------------
// HostFileSystem::Node::ResolvePath (private)
//
// Resolves a FileSystem::Alias from a relative object path
//
// Arguments:
//
//	path		- Relative file system object path string

FileSystem::AliasPtr HostFileSystem::Node::ResolvePath(const tchar_t* path)
{
	// Cannot resolve a null path
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// No need for recursion/searching for host file systems, just attempt to
	// create a new node instance from the combined base and relative path
	return HostFileSystem::NodeFromPath(m_mountpoint, AppendToPath(path));
}

bool HostFileSystem::Node::TryResolvePath(const tchar_t* path, FileSystem::AliasPtr& alias)
{
	// Cannot resolve a null path
	if(path == nullptr) return false;

	// TODO

	return false;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
