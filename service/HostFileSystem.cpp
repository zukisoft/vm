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
// HostFileSystem::AllocateNodeIndex
//
// Allocates a node index from the pool
//
// Arguments:
//
//	NONE

int32_t HostFileSystem::AllocateNodeIndex(void)
{
	int32_t index;				// Allocated index value

	// Try to use a spent index first before grabbing a new one; if the
	// return value overflowed, there are no more indexes left to use
	if(!m_spentindexes.try_pop(index)) index = m_nextindex++;
	if(index < 0) throw LinuxException(LINUX_EDQUOT);

	return index;
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount (static)
//
// Mounts the host file system by opening the specified directory
//
// Arguments:
//
//	device		- Path to the root file system node on the host

FileSystem::NodePtr HostFileSystem::Mount(const tchar_t* device)
{
	// The file system instance is required to exist as a shared_ptr
	auto fs = std::make_shared<HostFileSystem>();

	// Attempt to open the specified device as the host directory path, and
	// pass that into the special RootNode object.  RootNode contains a strong
	// reference to the file system parent to keep it alive
	HANDLE handle = OpenHostDirectory(device);
	try { return std::make_shared<RootNode>(fs, NodeType::Directory, handle); }
	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::OpenHostDirectory (static, private)
//
// Opens a directory object on the host file system
//
// Arguments:
//
//	path		- Path (host-relative) to the directory object to open

HANDLE HostFileSystem::OpenHostDirectory(const tchar_t* path)
{
	// NULL or zero-length path names are not supported, has to be set to something
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Attempt to open the specified path with query-only access to pass into the Node instance
	HANDLE handle = CreateFile(std::to_tstring(path).c_str(), 0, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	try {

		FILE_BASIC_INFO info;					// Information about the file system object

		// Query the basic information about the object to determine if it's a directory or not
		if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &info, sizeof(FILE_BASIC_INFO))) throw LinuxException(LINUX_EACCES, Win32Exception());
		if((info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);

		return handle;							// Return the operating system handle
	}

	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::OpenHostSymbolicLink (static, private)
//
// Opens a symbolic link object on the host file system
//
// Arguments:
//
//	path		- Path (host-relative) to the symbolic link object to open

HANDLE HostFileSystem::OpenHostSymbolicLink(const tchar_t* path)
{
	// NULL or zero-length path names are not supported, has to be set to something
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Attempt to open the specified path with query-only access to pass into the Node instance
	HANDLE handle = CreateFile(std::to_tstring(path).c_str(), 0, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OPEN_REPARSE_POINT, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	try {

		FILE_BASIC_INFO info;					// Information about the file system object

		// Query the basic information about the object to determine if it's a symbolic link or not
		if(!GetFileInformationByHandleEx(handle, FileBasicInfo, &info, sizeof(FILE_BASIC_INFO))) throw LinuxException(LINUX_EACCES, Win32Exception());
		if((info.FileAttributes & FILE_ATTRIBUTE_DIRECTORY) != FILE_ATTRIBUTE_REPARSE_POINT) throw LinuxException(LINUX_ENOTDIR);  // <-- TODO: correct error

		return handle;							// Return the operating system handle
	}

	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::ReleaseNodeIndex
//
// Releases a node index from the pool
//
// Arguments:
//
//	index		- Node index to be released

void HostFileSystem::ReleaseNodeIndex(int32_t index)
{
	// This class reuses indexes aggressively, push it into the spent queue
	// so that it be grabbed before allocating a new one
	m_spentindexes.push(index);
}

//
// TESTFILESYSTEM::NODE IMPLEMENTATION
//

//-----------------------------------------------------------------------------
// HostFileSystem::Node Constructor
//
// Arguments:
//
//	fs			- Reference to the parent file system instance
//	type		- Type associated with the node handle
//	handle		- Operating System handle for the node

HostFileSystem::Node::Node(const std::shared_ptr<HostFileSystem>& fs, NodeType type, HANDLE handle) : 
	m_type(type), m_handle(handle), m_fs(fs) 
{
	_ASSERTE(fs);								// The parent must be alive during construction
	_ASSERTE(handle != INVALID_HANDLE_VALUE);	// This must be set to something valid

	// Allocate the index for this node from the parent file system instance
	m_index = fs->AllocateNodeIndex();
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node Destructor

HostFileSystem::Node::~Node()
{
	// Close the underlying operating system handle
	if(m_handle != INVALID_HANDLE_VALUE) CloseHandle(m_handle);

	// Relase the index that was allocated for this node instance
	if(auto fs = m_fs.lock()) fs->ReleaseNodeIndex(m_index);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::CreateDirectory
//
// Creates a new directory node as a child of this node
//
// Arguments:
//
//	name		- Name to assign when creating the new node
//	mode		- Mode flags for the new node

FileSystem::NodePtr HostFileSystem::Node::CreateDirectory(const tchar_t* name, uapi::mode_t mode)
{
	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	_ASSERTE((name) && (*name));

	// The parent file system instance must be accessible during node construction
	auto fs = m_fs.lock();
	if(fs == nullptr) throw LinuxException(LINUX_ENOENT);

	// If this isn't a directory node, a new node cannot be created underneath it
	if(m_type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// The name must be non-NULL when creating a child node object
	if((name == nullptr) || (*name == 0)) throw LinuxException(LINUX_EINVAL);
	size_t namelen = _tcslen(name);

	// Force the mode to represent a directory object
	mode = (mode & ~LINUX_S_IFMT) | LINUX_S_IFDIR;

	// Determine the amount of space that needs to be allocated for the directory path name string; when 
	// providing NULL for the output, this will include the count for the NULL terminator
	size_t pathlen = GetFinalPathNameByHandle(m_handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

	// Retrieve the path to the directory object based on the handle
	std::vector<tchar_t> buffer(pathlen + 1 + namelen);
	pathlen = GetFinalPathNameByHandle(m_handle, buffer.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

	// Append a path separator character and copy in the new node name
	buffer[pathlen] = _T('\\');
	_tcsncat_s(buffer.data(), buffer.size(), name, namelen);

	// TODO: SECURITY_ATTRIBUTES based on mode
	if(!::CreateDirectory(buffer.data(), nullptr)) {
		
		DWORD result = GetLastError();						// Get the windows error that occurred

		// The exception that can be thrown differs based on the result from ::CreateDirectory()
		if(result == ERROR_ALREADY_EXISTS) throw LinuxException(LINUX_EEXIST, Win32Exception(result));
		else if(result == ERROR_PATH_NOT_FOUND) throw LinuxException(LINUX_ENOENT, Win32Exception(result));
		else throw LinuxException(LINUX_EINVAL, Win32Exception(result));
	}

	// The handle to the directory is not returned, it must be opened specifically for use
	// by the Node instance ...
	HANDLE handle = OpenHostDirectory(buffer.data());
	try { return std::make_shared<Node>(fs, NodeType::Directory, handle); }
	catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------
// HostFileSystem::Node::CreateSymbolicLink
//
// Creates a new symbolic link node as a child of this node
//
// Arguments:
//
//	name		- Name to assign when creating the new node
//	mode		- Mode flags for the new node

FileSystem::NodePtr HostFileSystem::Node::CreateSymbolicLink(const tchar_t* name, const tchar_t* target)
{
	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
	_ASSERTE((name) && (*name));
	_ASSERTE((target) && (*target));

	// The parent file system instance must be accessible during node construction
	auto fs = m_fs.lock();
	if(fs == nullptr) throw LinuxException(LINUX_ENOENT);

	// If this isn't a directory node, a new node cannot be created underneath it
	if(m_type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// The name must be non-NULL when creating a child node object
	if((name == nullptr) || (*name == 0)) throw LinuxException(LINUX_EINVAL);
	size_t namelen = _tcslen(name);

	// Determine the amount of space that needs to be allocated for the directory path name string; when 
	// providing NULL for the output, this will include the count for the NULL terminator
	size_t pathlen = GetFinalPathNameByHandle(m_handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

	// Retrieve the path to the directory object based on the handle
	std::vector<tchar_t> buffer(pathlen + 1 + namelen);
	pathlen = GetFinalPathNameByHandle(m_handle, buffer.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());

	// Append a path separator character and copy in the new node name
	buffer[pathlen] = _T('\\');
	_tcsncat_s(buffer.data(), buffer.size(), name, namelen);

	// TODO: adjust target for host
	// todo: flags, 0 = file
	// todo: exception codes
	if(!::CreateSymbolicLink(buffer.data(), target, 0)) throw LinuxException(LINUX_EINVAL, Win32Exception());

	return nullptr;
	// todo
	//// The handle to the directory is not returned, it must be opened specifically for use
	//// by the Node instance ...
	//HANDLE handle = OpenHostDirectory(buffer.data());
	//try { return std::make_shared<Node>(fs, NodeType::Directory, handle); }
	//catch(...) { CloseHandle(handle); throw; }
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
