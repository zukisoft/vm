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
//	superblock	- Reference to the superblock instance for this file system
//	root		- Pointer to the root node for the file system

HostFileSystem::HostFileSystem(const std::shared_ptr<SuperBlock>& superblock, const std::shared_ptr<Node>& root) 
	: m_superblock(superblock), m_root(root)
{
	_ASSERTE(superblock);
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
	// TODO: Want special flag for preventing cross-mount access

	// need: node index allocation

	// Determine the type of node that the path represents; must be a directory for mounting
	FileSystem::NodeType type = NodeTypeFromPath(source);
	if(type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// Attempt to open a query-only handle against the file system object
	HANDLE handle = CreateFile(source, 0, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	try {

		// Determine the amount of space that needs to be allocated for the canonicalized path name string; when 
		// providing NULL for the output, this will include the count for the NULL terminator
		uint32_t pathlen = GetFinalPathNameByHandle(handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

		// Retrieve the canonicalized path to the directory object based on the handle; this will serve as the base
		std::vector<tchar_t> path(pathlen);
		pathlen = GetFinalPathNameByHandle(handle, path.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
		if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

		// Construct the superblock that will be used for this instance of the file system
		std::shared_ptr<SuperBlock> superblock = std::make_shared<SuperBlock>(path, flags, data);

		// Construct the HostFileSystem instance as well as the root node object
		return std::make_shared<HostFileSystem>(superblock, std::make_shared<Node>(superblock, std::move(path), type, handle));
	} 
	
	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeFromPath (private, static)
//
// Creates a HostFileSystem::Node instance based on a host path
//
// Arguments:
//
//	superblock	- Reference to the superblock instance for this file system
//	path		- Host operating system path to construct the node against
//	follow		- Flag to follow the final path component if a symbolic link

std::shared_ptr<HostFileSystem::Node> HostFileSystem::NodeFromPath(const std::shared_ptr<SuperBlock>& superblock, 
	const tchar_t* path, bool follow)
{
	_ASSERTE(path);
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Convert the path string into a vector<> that can be moved into the Node instance
	std::vector<tchar_t> pathvec(_tcslen(path) + 1);
	memcpy(pathvec.data(), path, pathvec.size() * sizeof(tchar_t));

	return NodeFromPath(superblock, std::move(pathvec), follow);
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeFromPath (private, static)
//
// Creates a HostFileSystem::Node instance based on a host path
//
// Arguments:
//
//	superblock	- Reference to the superblock instance for this file system
//	path		- Host operating system path to construct the node against
//	follow		- Flag to follow the final path component if a symbolic link

std::shared_ptr<HostFileSystem::Node> HostFileSystem::NodeFromPath(const std::shared_ptr<SuperBlock>& superblock, 
	std::vector<tchar_t>&& path, bool follow)
{
	DWORD				attributes;							// Windows file/directory attributes
	DWORD				flags = FILE_ATTRIBUTE_NORMAL;		// CreateFile() flags

	// Determine the attributes and type of node that the path represents; throws if path is bad
	FileSystem::NodeType type = NodeTypeFromPath(path.data(), &attributes);

	// Directories require FILE_FLAG_BACKUP_SEMANTICS to be set
	if(attributes & FILE_ATTRIBUTE_DIRECTORY) flags |= FILE_FLAG_BACKUP_SEMANTICS;

	// If not following a final symbolic link, set FILE_FLAG_OPEN_REPARSE_POINT to access it
	if(!follow) flags |= FILE_FLAG_OPEN_REPARSE_POINT;

	// Attempt to open a query-only handle against the file system object
	HANDLE handle = CreateFile(path.data(), 0, 0, nullptr, OPEN_EXISTING, flags, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	try { 

		// Validate that the handle meets the necessary criteria for this mount
		superblock->ValidateHandle(handle);

		// Generate and return the new Node instance
		return std::make_shared<Node>(superblock, std::move(path), type, handle); 
	}

	catch(...) { CloseHandle(handle); throw; }
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

	// Query the basic attributes about the specified path
	DWORD attrs = GetFileAttributes(path);
	if(attrs == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOENT, Win32Exception());

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

HostFileSystem::Handle::Handle(const std::shared_ptr<SuperBlock>& superblock, const std::shared_ptr<Node>& node, HANDLE handle, int flags)
	: m_alignment(1), m_superblock(superblock), m_node(node), m_handle(handle), m_flags(flags)
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
// HOSTFILESYSTEM::NODE
//

//-----------------------------------------------------------------------------
// HostFileSystem::Node Constructor
//
// Arguments:
//
//	superblock	- Reference to the mounted file system superblock
//	path		- vector<> containing the operating system path
//	type		- Type of node being constructed
//	handle		- Open operating system handle for the node (query access)

HostFileSystem::Node::Node(const std::shared_ptr<SuperBlock>& superblock, std::vector<tchar_t>&& path, FileSystem::NodeType type, HANDLE handle)
	: m_superblock(superblock), m_path(std::move(path)), m_type(type), m_handle(handle)
{
	_ASSERTE(handle != INVALID_HANDLE_VALUE);

	// Get a pointer to the path leaf name to satisfy the FileSystem::Alias interface
	m_name = PathFindFileName(m_path.data());
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
	if(m_superblock->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

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

//-----------------------------------------------------------------------------
// HostFileSystem::Node::CreateSymbolicLink (private)
//
// Creates a new symbolic link node as a child of this node
//
// Arguments:
//
//	name		- Name to assign to the new symbolic link
//	target		- Symbolic link target path; must be within this file system

void HostFileSystem::Node::CreateSymbolicLink(const tchar_t* name, const tchar_t* target)
{
	// TODO: This is actually going to be a little complicated.  The target needs
	// to be adjusted such that it's relative to the mount point for this
	// HostFileSystem instance, cannot allow links outside of this since people
	// could get outside the mounted file system root, and that would be bad.

	// there is also a problem with deciding if a symlink should be a directory link
	// or a file link, sadly Windows needs to know this

	// It may be best to not support this at all for HostFileSystem, they can't be
	// created or opened unless the user is an administrator anyway.  it would be
	// a simple matter to just ignore the 'follow' flag passed into ResolvePath.

	(name);
	(target);
	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
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
	if((m_superblock->Options.ReadOnly) && (access & GENERIC_WRITE)) throw LinuxException(LINUX_EROFS);

	// Convert the file control flags into a disposition code (create/open/truncate/etc)
	DWORD disposition = OPEN_EXISTING;
	if((flags & (LINUX_O_CREAT | LINUX_O_EXCL)) == (LINUX_O_CREAT | LINUX_O_EXCL)) disposition = CREATE_NEW;
	else if(flags & LINUX_O_CREAT) disposition = OPEN_ALWAYS;

	// TODO: O_TRUNC - File needs to exist and be a regular for this to work, otherwise this is ignored
	// (it would become TRUNCATE_EXISTING for disposition)

	DWORD attributes = FILE_ATTRIBUTE_NORMAL | FILE_FLAG_POSIX_SEMANTICS;
	// todo: BACKUP_SEMANTICS
	// todo: OPEN_REPARSE_POINT
	if(flags & LINUX_O_SYNC) attributes |= FILE_FLAG_WRITE_THROUGH;
	if(flags & LINUX_O_DIRECT) attributes |= FILE_FLAG_NO_BUFFERING;

	HANDLE handle = CreateFile(m_path.data(), access, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, disposition, attributes, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EPERM);	// <--- todo

	try { return std::make_shared<Handle>(m_superblock, shared_from_this(), handle, flags); }
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
//	follow		- Flag to follow the final path component if a symbolic link

FileSystem::AliasPtr HostFileSystem::Node::ResolvePath(const tchar_t* path, bool follow)
{
	// Cannot resolve a null path
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// No need for recursion/searching for host file systems, just attempt to
	// create a new node instance from the combined base and relative path
	return HostFileSystem::NodeFromPath(m_superblock, AppendToPath(path), follow);
}

//
// HOSTFILESYSTEM::SUPERBLOCK
//

//-----------------------------------------------------------------------------
// HostFileSystem::SuperBlock::ValidateHandle
//
// Validates that a newly opened file system handle meets the criteria set
// for this host mount point
//
// Arguments:
//
//	handle		- Operating system handle to validate

void HostFileSystem::SuperBlock::ValidateHandle(HANDLE handle)
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

//-----------------------------------------------------------------------------

#pragma warning(pop)
