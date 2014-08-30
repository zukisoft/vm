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
// HostFileSystem::MapException (private, static)
//
// Converts a Win32 error code into a representative LinuxException instance
//
// Arguments:
//
//	code		- Win32 error code to be mapped

LinuxException HostFileSystem::MapException(DWORD code)
{
	int linuxcode = LINUX_EIO;				// Use EIO as the default linux error code

	// todo: this is for testing the codes only, remove me
	DebugBreak();

	// Try to map the Win32 error code to something that makes sense in the context of a file system
	switch(code) {

		case ERROR_FILE_NOT_FOUND : linuxcode = LINUX_ENOENT; break;
		case ERROR_PATH_NOT_FOUND : linuxcode = LINUX_ENOENT; break;
		case ERROR_FILE_EXISTS : linuxcode = LINUX_EEXIST; break;
		case ERROR_INVALID_PARAMETER : linuxcode = LINUX_EINVAL; break;
		case ERROR_ALREADY_EXISTS : linuxcode = LINUX_EEXIST; break;
	}

	// Generate a LinuxException with the mapped code and provide the underlying Win32
	// error as an inner Win32Exception instance
	return LinuxException(linuxcode, Win32Exception(code));
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

	// Determine the type of node that the path represents; must be a directory for mounting
	DWORD attributes = GetFileAttributes(source);
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOTDIR);
	if((attributes & FILE_ATTRIBUTE_DIRECTORY) == 0) throw LinuxException(LINUX_ENOTDIR);

	// Attempt to open a query-only handle against the file system directory object
	HANDLE handle = CreateFile(source, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, FILE_FLAG_POSIX_SEMANTICS | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw HostFileSystem::MapException();

	// Attempt to create the MountPoint instance for this file system against the handle
	try { mountpoint = std::make_shared<MountPoint>(handle, flags, data); }
	catch(...) { CloseHandle(handle); throw; }

	// Construct the HostFileSystem instance with a root Node instance that references the base path
	return std::make_shared<HostFileSystem>(mountpoint, Node::Construct(mountpoint, mountpoint->BasePath));
}

//
// HOSTFILESYSTEM::HANDLE
//

//-----------------------------------------------------------------------------
// HostFileSystem::Handle Constructor
//
// Arguments:
//
//	mountpoint		- Reference to the HostFileSystem mountpoint object
//	node			- Parent Node instance for this Handle instance
//	handle			- Operating system handle to be wrapped
//	flags			- Linux flags used when opening the operating system handle

HostFileSystem::Handle::Handle(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Node>& node, HANDLE handle, int flags)
	: m_mountpoint(mountpoint), m_node(node), m_handle(handle), m_flags(flags)
{
	FILE_STORAGE_INFO			storageinfo;			// Information about underyling storage

	// Retrieve the file alignment information from the operating system
	if(!GetFileInformationByHandleEx(handle, FileStorageInfo, &storageinfo, sizeof(storageinfo))) throw HostFileSystem::MapException();
	m_alignment = storageinfo.FileSystemEffectivePhysicalBytesPerSectorForAtomicity;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Handle Destructor

HostFileSystem::Handle::~Handle()
{
	// Close the underlying operating system handle for this object
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Handle::Read (private)
//
// Synchronously reads data from the underlying object
//
// Arguments:
//
//	buffer			- Buffer to read the object data into
//	count			- Number of bytes to be read from the file

uapi::size_t HostFileSystem::Handle::Read(void* buffer, uapi::size_t count)
{
	DWORD				read;			// Bytes read from the file

	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);
	if(m_handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF);

	// Host file system does not support reading from directories or symbolic links
	if(m_node->Type == FileSystem::NodeType::Directory) throw LinuxException(LINUX_EISDIR);
	else if(m_node->Type != FileSystem::NodeType::File) throw LinuxException(LINUX_EINVAL);

	// If the handle was opened with O_DIRECT, the buffer, count and current offset must be aligned
	if(m_flags & LINUX_O_DIRECT) ValidateAlignment(buffer, count);

	// Attempt to read the specified number of bytes from the file into the buffer
	if(!ReadFile(m_handle, buffer, count, &read, nullptr)) throw HostFileSystem::MapException();

	return static_cast<uapi::size_t>(read);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Handle::Sync (private)
//
// Synchronizes any operating system buffers or caches associated with the handle
//
// Arguments:
//
//	NONE

void HostFileSystem::Handle::Sync(void)
{
	if(m_handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF);

	// The closest equivalent of fsync/fdatasync on Windows is FlushFileBuffers()
	if(!FlushFileBuffers(m_handle)) throw HostFileSystem::MapException();
}

//-----------------------------------------------------------------------------
// HostFileSystem::Handle::Write (private)
//
// Writes data to the underlying file system object at the current position
//
// Arguments:
//
//	buffer			- Pointer to the buffer containing the source data
//	count			- Size of the input buffer, in bytes

uapi::size_t HostFileSystem::Handle::Write(const void* buffer, uapi::size_t count)
{
	DWORD				written;		// Bytes written to the file

	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);
	if(m_handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF);

	// Host file system does not support writing to directories or symbolic links
	if(m_node->Type == FileSystem::NodeType::Directory) throw LinuxException(LINUX_EISDIR);
	else if(m_node->Type != FileSystem::NodeType::File) throw LinuxException(LINUX_EINVAL);

	// There is no way to make O_APPEND atomic on the host file system, just move it and hope for the best
	if((m_flags & LINUX_O_APPEND) && (SetFilePointer(m_handle, 0, 0, FILE_END) == INVALID_SET_FILE_POINTER))
		throw HostFileSystem::MapException();

	// If the handle was opened with O_DIRECT, the buffer, count and current offset must be aligned
	if(m_flags & LINUX_O_DIRECT) ValidateAlignment(buffer, count);

	// Attempt to write the specified number of bytes from the buffer into the file
	if(!WriteFile(m_handle, buffer, count, &written, nullptr)) throw HostFileSystem::MapException();

	return static_cast<uapi::size_t>(written);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Handle::ValidateAlignment (private)
//
// Used with O_DIRECT, this validates that the memory buffer pointer as well as
// the current operating system file pointer have the proper alignment
//
// Arguments:
//
//	buffer		- Pointer to the input/output buffer
//	count		- Size of the input/output buffer, in bytes

void HostFileSystem::Handle::ValidateAlignment(const void* buffer, uapi::size_t count)
{
	LARGE_INTEGER pointer = { 0, 0 };

	if(!buffer) throw LinuxException(LINUX_EINVAL, Exception(E_POINTER));

	// Retrieve the current file pointer offset so that it can be validated with the other criteria
	if(!SetFilePointerEx(m_handle, pointer, &pointer, FILE_CURRENT)) throw HostFileSystem::MapException();

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
	// Determine the amount of space that needs to be allocated for the canonicalized path name string; when 
	// providing NULL for the output, this will include the count for the NULL terminator
	uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw HostFileSystem::MapException();

	// Retrieve the canonicalized path to the directory object based on the handle; this will serve as the base
	m_path.resize(pathlen);
	pathlen = GetFinalPathNameByHandle(handle, m_path.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw HostFileSystem::MapException();
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
		if(pathlen == 0) throw HostFileSystem::MapException();

		// If the path is at least as long as the original mountpoint name, it cannot be a child
		if(pathlen < m_path.size()) throw LinuxException(LINUX_EPERM);

		// Retrieve the path to the directory object based on the handle
		std::vector<tchar_t> path(pathlen);
		pathlen = GetFinalPathNameByHandle(handle, path.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if(pathlen == 0) throw HostFileSystem::MapException();

		// Verify that the canonicalized path starts with the mount point, don't ignore case
		if(_tcsncmp(m_path.data(), path.data(), m_path.size() - 1) != 0) throw LinuxException(LINUX_EPERM);
	}
}

//
// HOSTFILESYSTEM::NODE
//

//-----------------------------------------------------------------------------
// HostFileSystem::Node Constructor (private)
//
// Arguments:
//
//	mountpoint	- Reference to the file system mountpoint instance
//	path		- vector<> containing the operating system path
//	handle		- Open operating system handle for the node (query access)

HostFileSystem::Node::Node(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path, HANDLE handle)
	: m_mountpoint(mountpoint), m_path(std::move(path)), m_handle(handle)
{
	// Get a pointer to the path leaf name to satisfy the FileSystem::Alias interface
	m_name = PathFindFileName(m_path.data());

	// Retrieve the basic information about this node from the operating system
	if(!GetFileInformationByHandle(handle, &m_info)) throw HostFileSystem::MapException();

	// Convert the operating system node attributes into FileSystem::NodeType
	if(m_info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) m_type = FileSystem::NodeType::SymbolicLink;
	else if(m_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) m_type = FileSystem::NodeType::Directory;
	else m_type = FileSystem::NodeType::File;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node Destructor

HostFileSystem::Node::~Node()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
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
// HostFileSystem::Node::Construct (static)
//
// Creates a new Node instance
//
// Arguments:
//
//	mountpoint		- Reference to the file system mount point
//	path			- Operating system path on which to construct the Node

std::shared_ptr<HostFileSystem::Node> HostFileSystem::Node::Construct(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path)
{
	// The provided path string must not be null or zero-length
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Convert the path string into a vector<> that can be moved into the Node instance
	std::vector<tchar_t> pathvec(_tcslen(path) + 1);
	memcpy(pathvec.data(), path, pathvec.size() * sizeof(tchar_t));

	// Invoke the version of this method that accepts the vector<> rvalue reference
	return Construct(mountpoint, std::move(pathvec));
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::Construct (static)
//
// Creates a new Node instance
//
// Arguments:
//
//	mountpoint		- Reference to the file system mount point
//	path			- Operating system path on which to construct the Node

std::shared_ptr<HostFileSystem::Node> HostFileSystem::Node::Construct(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path)
{
	// The node type must be known in order to open the basic handle
	DWORD attributes = GetFileAttributes(path.data());
	if(attributes == INVALID_FILE_ATTRIBUTES) throw HostFileSystem::MapException();

	// Construct the flags to be passed to CreateFile() to deal with directory and symbolic links
	DWORD flags = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS;
	if(attributes & FILE_ATTRIBUTE_DIRECTORY) flags |= FILE_FLAG_BACKUP_SEMANTICS;
	if(attributes & FILE_ATTRIBUTE_REPARSE_POINT) flags |= FILE_FLAG_OPEN_REPARSE_POINT;

	// Attempt to create a query-only handle for the underlying operating system object
	HANDLE handle = ::CreateFile(path.data(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, flags, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw HostFileSystem::MapException();

	// Invoke the version of this method that accepts the vector<> rvalue reference and the handle
	try { return Construct(mountpoint, std::move(path), handle); }
	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::Construct (static)
//
// Creates a new Node instance
//
// Arguments:
//
//	mountpoint		- Reference to the file system mount point
//	handle			- Operating system handle (query access) to assign to the Node

std::shared_ptr<HostFileSystem::Node> HostFileSystem::Node::Construct(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle)
{
	// Determine the amount of space that needs to be allocated for the canonicalized path name string; when 
	// providing NULL for the output, this will include the count for the NULL terminator
	uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw HostFileSystem::MapException();

	// Retrieve the canonicalized path to the directory object based on the handle
	std::vector<tchar_t> pathvec(pathlen);
	pathlen = GetFinalPathNameByHandle(handle, pathvec.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw HostFileSystem::MapException();

	// Invoke the version of this method that accepts the vector<> rvalue reference
	return Construct(mountpoint, std::move(pathvec), handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::Construct (static)
//
// Creates a new Node instance
//
// Arguments:
//
//	mountpoint		- Reference to the file system mount point
//	path			- Operating system path on which to construct the Node
//	handle			- Operating system handle (query access) to assign to the Node

std::shared_ptr<HostFileSystem::Node> HostFileSystem::Node::Construct(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* path, HANDLE handle)
{
	// The provided path string must not be null or zero-length
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Convert the path string into a vector<> that can be moved into the Node instance
	std::vector<tchar_t> pathvec(_tcslen(path) + 1);
	memcpy(pathvec.data(), path, pathvec.size() * sizeof(tchar_t));

	// Invoke the version of this method that accepts the vector<> rvalue reference
	return Construct(mountpoint, std::move(pathvec), handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::Construct (static)
//
// Creates a new Node instance
//
// Arguments:
//
//	mountpoint		- Reference to the file system mount point
//	path			- Operating system path on which to construct the Node
//	handle			- Operating system handle (query access) to assign to the Node

std::shared_ptr<HostFileSystem::Node> HostFileSystem::Node::Construct(const std::shared_ptr<MountPoint>& mountpoint, std::vector<tchar_t>&& path, HANDLE handle)
{
	// This method is the final link in the Construct() chain; just create the Node instance
	return std::make_shared<Node>(mountpoint, std::move(path), handle);
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
	if(!::CreateDirectory(pathvec.data(), nullptr)) throw HostFileSystem::MapException();
}

// todo: need mode
FileSystem::HandlePtr HostFileSystem::Node::CreateFile(const tchar_t* name, int flags)
{
	// Cannot create a file with a null or zero-length name
	if((name == nullptr) || (*name == 0)) throw LinuxException(LINUX_EINVAL);

	// Check that the file system is not mounted as read-only
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// The host file system cannot support unnamed temporary files via O_TMPFILE
	if(flags & LINUX___O_TMPFILE) throw LinuxException(LINUX_EINVAL);

	// Combine the name with the base path for this node
	std::vector<tchar_t> pathvec = AppendToPath(name);

	// TODO: build attributes, O_TRUNC, O_SYNC, O_DIRECT, etc
	DWORD attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS;

	// Attempt to create a new file object on the host file system with the specified attributes
	HANDLE handle = ::CreateFile(pathvec.data(), FlagsToAccess(flags), FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CREATE_NEW, attributes, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw HostFileSystem::MapException();

	try {

		// In order to construct the Node instance atomically for the newly created file, use ReOpenFile
		// to generate a clone of the handle rather than closing it
		HANDLE queryhandle = ReOpenFile(handle, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, FILE_FLAG_POSIX_SEMANTICS);
		if(queryhandle == INVALID_HANDLE_VALUE) throw HostFileSystem::MapException();

		// Construct a Node instance to serve as the parent object for the Handle instance
		std::shared_ptr<Node> node = Node::Construct(m_mountpoint, std::move(pathvec), queryhandle);

		// Construct and return a new Handle instance to the caller
		try { return Handle::Construct(m_mountpoint, node, handle, flags); }
		catch(...) { CloseHandle(queryhandle); throw; }
	}

	catch(...) { CloseHandle(handle); throw; }
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
// Creates a FileSystem::Handle instance from this node
//
// Arguments:
//
//	flags		- Open flags from the caller

FileSystem::HandlePtr HostFileSystem::Node::OpenHandle(int flags)
{
	// The host file system cannot support unnamed temporary files via O_TMPFILE
	if(flags & LINUX___O_TMPFILE) throw LinuxException(LINUX_EINVAL);

	// Convert the file control flags into the access flags and check for read-only file system
	DWORD access = FlagsToAccess(flags);
	if((m_mountpoint->Options.ReadOnly) && (access & GENERIC_WRITE)) throw LinuxException(LINUX_EROFS);

	//
	// TODO: file must exist to be opened here, check for O_CREAT | O_EXCL and fail
	//

	// Generate the attributes for the open operation based on the node type and provided flags
	DWORD attributes = FILE_FLAG_POSIX_SEMANTICS;
	if(m_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) attributes |= FILE_FLAG_BACKUP_SEMANTICS;
	if(m_info.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) attributes |= FILE_FLAG_OPEN_REPARSE_POINT;
	if(flags & LINUX_O_SYNC) attributes |= FILE_FLAG_WRITE_THROUGH;
	if(flags & LINUX_O_DIRECT) attributes |= FILE_FLAG_NO_BUFFERING;

	// Use the contained query-only handle to reopen the file with the requested attributes
	HANDLE handle = ReOpenFile(m_handle, access, FILE_SHARE_READ | FILE_SHARE_WRITE, attributes);
	if(handle == INVALID_HANDLE_VALUE) throw HostFileSystem::MapException();

	try {

		// By using ReOpenFile() rather than CreateFile() to generate the handle, there is no opportunity
		// to specify TRUNCATE_EXISTING in the disposition flags, it must be done after the fact
		if((access & GENERIC_WRITE) && (flags & LINUX_O_TRUNC)) { if(!SetEndOfFile(handle)) throw HostFileSystem::MapException(); }

		// Generate a new Handle instance around the new object handle and flags
		return Handle::Construct(m_mountpoint, shared_from_this(), handle, flags);
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
	return Node::Construct(m_mountpoint, AppendToPath(path));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
