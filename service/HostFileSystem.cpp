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

// INVARIANTS
//
static_assert(FILE_BEGIN == LINUX_SEEK_SET,		"HostFileSystem: FILE_BEGIN must be the same value as LINUX_SEEK_SET");
static_assert(FILE_CURRENT == LINUX_SEEK_CUR,	"HostFileSystem: FILE_CURRENT must be the same value as LINUX_SEEK_CUR");
static_assert(FILE_END == LINUX_SEEK_END,		"HostFileSystem: FILE_END must be the same value as LINUX_SEEK_END");

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
	// (MountPoint will take ownership of the handle at this point)
	try { mountpoint = std::make_shared<MountPoint>(handle, flags, data); }
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
	_ASSERTE(name);
	_ASSERTE(node);

	if((!name) || (!node)) throw LinuxException(LINUX_EINVAL);

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

void HostFileSystem::DirectoryNode::CreateDirectory(const FileSystem::AliasPtr& parent, const uapi::char_t* name)
{
	(parent);
	(name);
	throw Exception(E_NOTIMPL);
}

FileSystem::HandlePtr HostFileSystem::DirectoryNode::CreateFile(const FileSystem::AliasPtr& parent, const uapi::char_t* name, int flags)
{
	(parent);
	(name);
	(flags);
	throw Exception(E_NOTIMPL);
}

void HostFileSystem::DirectoryNode::CreateSymbolicLink(const FileSystem::AliasPtr&, const uapi::char_t*, const uapi::char_t*) 
{ 
	// Creation of symbolic links is not supported via HostFileSystem
	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL)); 
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
	_ASSERTE(handle != INVALID_HANDLE_VALUE);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT);

	// When a handle is provided, just invoke the constructor
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

FileSystem::HandlePtr HostFileSystem::DirectoryNode::Open(int flags)
{
	(flags);
	throw Exception(E_NOTIMPL);
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
	FileSystem::AliasPtr		resolved;						// Alias instance resolved from the host path

	if(path == nullptr) throw LinuxException(LINUX_ENOENT);		// NULL path strings are disallowed

	(flags); // TODO - WORK IN PROGRESS -- NEED TO IMPLEMENT THESE -- SEE TEMPFILESYSTEM

	// If a blank path has been specified, the resolution process ends at this node
	if(path[0] == '\0') return current;

	std::tstring pathstr = std::to_tstring(path);				// Convert relative path from ANSI/UTF-8

	// Combine the provided path with the stored path to complete the path to the target node
	HRESULT hresult = PathAllocCombine(m_hostpath, pathstr.c_str(), PATHCCH_ALLOW_LONG_PATHS, &hostpath);
	if(FAILED(hresult)) throw Exception(hresult);			// <--- todo linux exception

	// Extract the file name from the path and convert it to ANSI/UTF-8 for the alias instance
	std::string aliasname = std::to_string(PathFindFileName(hostpath));

	// Retrieve the basic attributes for the node to determine if it's a file or a directory
	DWORD attributes = GetFileAttributes(hostpath);
	if(attributes == INVALID_FILE_ATTRIBUTES) { LocalFree(hostpath); throw LinuxException(LINUX_ENOENT); }

	try {

		// Generate either a directory or file alias based on what the underlying object type happens to be
		if((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) 
			resolved = Alias::Construct(aliasname.c_str(), DirectoryNode::FromPath(m_mountpoint, hostpath));
		else resolved = Alias::Construct(aliasname.c_str(), FileNode::FromPath(m_mountpoint, hostpath));

		LocalFree(hostpath);									// Release allocated string data
		return resolved;										// Return the resolved Alias instance
	}
	
	catch(...) { LocalFree(hostpath); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::getIndex
//
// Gets the file index for this node from the operating system

uint64_t HostFileSystem::DirectoryNode::getIndex(void)
{
	return HandleToIndex(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::DirectoryNode::getType
//
// Gets the node type for this object

FileSystem::NodeType HostFileSystem::DirectoryNode::getType(void)
{
	return FileSystem::NodeType::Directory;
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::DIRECTORYNODE::HANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

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
	_ASSERTE(handle != INVALID_HANDLE_VALUE);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT);

	// When a handle is provided, just invoke the constructor
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

FileSystem::HandlePtr HostFileSystem::FileNode::Open(int flags)
{
	(flags);
	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

FileSystem::HandlePtr HostFileSystem::FileNode::OpenExec(int flags)
{
	(flags);
	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
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

uint64_t HostFileSystem::FileNode::getIndex(void)
{
	return HandleToIndex(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::getType
//
// Gets the node type for this object

FileSystem::NodeType HostFileSystem::FileNode::getType(void)
{
	return FileSystem::NodeType::File;
}

//-----------------------------------------------------------------------------
// HOSTFILESYSTEM::FILENODE::EXECHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::ExecHandle Constructor
//
// Arguments:
//
//	mountpoint		- Mounted file system metadata and options
//	handle			- Handle to the underlying file object

HostFileSystem::FileNode::ExecHandle::ExecHandle(const std::shared_ptr<MountPoint>& mountpoint, HANDLE handle) : m_mountpoint(mountpoint)
{
	_ASSERTE(handle != INVALID_HANDLE_VALUE);
	
	// TODO: CALL DUPLICATE HANDLE TO GENERATE THE NEW HANDLE
	m_handle = INVALID_HANDLE_VALUE;
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::ExecHandle Destructor

HostFileSystem::FileNode::ExecHandle::~ExecHandle()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::ExecHandle::Read
//
// Reads data from the current file pointer into an output buffer
//
// Arguments:
//
//	buffer		- Pointer to the output buffer
//	count		- Number of bytes to read from the file, also minimum size of buffer

uapi::size_t HostFileSystem::FileNode::ExecHandle::Read(void* buffer, uapi::size_t count)
{
	DWORD				read;			// Bytes read from the file

	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);
	if(m_handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF, Exception(E_HANDLE));

	// ReadFile() can only read up to MAXDWORD bytes from the underlying file
	if(count >= MAXDWORD) throw LinuxException(LINUX_EINVAL, Exception(E_BOUNDS));

	// Attempt to read the specified number of bytes from the file into the buffer
	if(!ReadFile(m_handle, buffer, static_cast<DWORD>(count), &read, nullptr)) throw MapHostException();

	return static_cast<uapi::size_t>(read);
}

//-----------------------------------------------------------------------------
// HostFileSystem::FileNode::ExecHandle::Seek
//
// Sets the file pointer for this handle
//
// Arguments:
//
//	offset		- Offset into the file to set the pointer, based on whence
//	whence		- Starting position within the file to apply offset

uapi::loff_t HostFileSystem::FileNode::ExecHandle::Seek(uapi::loff_t offset, int whence)
{
	LARGE_INTEGER			position;			// Distance to be moved

	if(m_handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_EBADF, Exception(E_HANDLE));

	// This is a simple operation on the host, provided the values for whence mean the same
	// thing, which is checked as an invariant at the top of this file
	position.QuadPart = offset;
	if(!SetFilePointerEx(m_handle, position, &position, whence)) throw MapHostException();

	return position.QuadPart;
}

void HostFileSystem::FileNode::ExecHandle::Sync(void)
{
	// Execute-only handles do not support synchronization
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

void HostFileSystem::FileNode::ExecHandle::SyncData(void)
{
	// Execute-only handles do not support synchronization
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

uapi::size_t HostFileSystem::FileNode::ExecHandle::Write(const void*, uapi::size_t)
{
	// Execute-only handles do not support write
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

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

HostFileSystem::MountPoint::MountPoint(HANDLE handle, uint32_t flags, const void* data) : 
	m_handle(handle), m_options(flags, data), m_hostpath(HandleToPath(handle))
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
// HOSTFILESYSTEM::PATHHANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#pragma warning(pop)
