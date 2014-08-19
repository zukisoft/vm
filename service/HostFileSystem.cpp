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
//	rootnode		- Pointer to the root node for the file system

HostFileSystem::HostFileSystem(const std::shared_ptr<Node>& rootnode) : m_rootnode(rootnode)
{
	_ASSERTE(rootnode);
}

//-----------------------------------------------------------------------------
// HostFileSystem::Mount (static)
//
// Mounts the host file system by opening the specified directory
//
// Arguments:
//
//	device		- Path to the root file system node on the host

FileSystemPtr HostFileSystem::Mount(const tchar_t* device)
{
	// Attempt to create the root node from the specified path; must be a directory
	std::shared_ptr<Node> rootnode = NodeFromPath(device);
	if(rootnode->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// todo: this will need volume and quota information eventually when the
	// superblock-style functions are written
	return std::make_shared<HostFileSystem>(rootnode);
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeFromPath (private, static)
//
// Creates a HostFileSystem::Node instance based on a host path
//
// Arguments:
//
//	path		- Host operating system path to construct the node against

std::shared_ptr<HostFileSystem::Node> HostFileSystem::NodeFromPath(const tchar_t* path)
{
	_ASSERTE(path);
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Convert the path string into a vector<> that can be moved into the Node instance
	std::vector<tchar_t> pathvec(_tcslen(path) + 1);
	memcpy(pathvec.data(), path, pathvec.size() * sizeof(tchar_t));

	return NodeFromPath(std::move(pathvec));
}

//-----------------------------------------------------------------------------
// HostFileSystem::NodeFromPath (private, static)
//
// Creates a HostFileSystem::Node instance based on a host path
//
// Arguments:
//
//	path		- Host operating system path to construct the node against

std::shared_ptr<HostFileSystem::Node> HostFileSystem::NodeFromPath(std::vector<tchar_t>&& path)
{
	// Determine the type of node that the path represents; throws if path is bad
	FileSystem::NodeType type = NodeTypeFromPath(path.data());

	// Different node types require different flags to CreateFile() in order to get the handle
	DWORD flags = FILE_ATTRIBUTE_NORMAL;
	if(type == FileSystem::NodeType::Directory) flags |= FILE_FLAG_BACKUP_SEMANTICS;
	else if(type == FileSystem::NodeType::SymbolicLink) flags |= FILE_FLAG_OPEN_REPARSE_POINT;

	// Attempt to open a query-only handle against the file system object
	HANDLE handle = CreateFile(path.data(), 0, 0, nullptr, OPEN_EXISTING, flags, nullptr);
	if(handle == INVALID_HANDLE_VALUE) throw LinuxException(LINUX_ENOENT, Win32Exception());

	// Return a new node instance that represents the path 
	try { return std::make_shared<Node>(std::move(path), type, handle); }
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

FileSystem::NodeType HostFileSystem::NodeTypeFromPath(const tchar_t* path)
{
	_ASSERTE(path);

	// Query the basic attributes about the specified path
	DWORD attributes = GetFileAttributes(path);
	if(attributes == INVALID_FILE_ATTRIBUTES) throw LinuxException(LINUX_ENOENT, Win32Exception());

	// Check for REPARSE_POINT first as it will be combined with other flags indicating if this is a directory or a file
	if((attributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) return FileSystem::NodeType::SymbolicLink;
	else if((attributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY) return FileSystem::NodeType::Directory;
	else return FileSystem::NodeType::File;
}

//
// HOSTFILESYSTEM::NODE
//

//-----------------------------------------------------------------------------
// HostFileSystem::Node Constructor
//
// Arguments:
//
//	path		- vector<> containing the operating system path
//	type		- Type of node being constructed
//	handle		- Open operating system handle for the node (query access)

HostFileSystem::Node::Node(std::vector<tchar_t>&& path, FileSystem::NodeType type, HANDLE handle)
	: m_path(std::move(path)), m_type(type), m_handle(handle)
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
// HostFileSystem::Node::ResolvePath (private)
//
// Resolves a FileSystem::Alias from a relative object path
//
// Arguments:
//
//	path		- Relative file system object path string

FileSystem::AliasPtr HostFileSystem::Node::ResolvePath(const tchar_t* path)
{
	_ASSERTE(path);
	if((path == nullptr) || (*path == 0)) throw LinuxException(LINUX_ENOENT);

	// Copy the path to this node and add space for a possible "\\?\" prefix in the
	// event that the combined path will require it
	std::vector<tchar_t> pathvec(m_path);
	pathvec.resize(pathvec.size() + _tcslen(path) + 5);

	// Append the provided path to this node's path
	HRESULT hresult = PathCchAppendEx(pathvec.data(), pathvec.size(), path, PATHCCH_ALLOW_LONG_PATHS);
	if(FAILED(hresult)) throw LinuxException(LINUX_ENAMETOOLONG, Exception(hresult));
	
	// Trim the excess from the end of the path string and create a new Node instance
	pathvec.resize(_tcslen(pathvec.data()) + 1);
	return HostFileSystem::NodeFromPath(std::move(pathvec));
}

//-----------------------------------------------------------------------------

//FileSystem::NodePtr HostFileSystem::Node::CreateDirectory(const tchar_t* name, uapi::mode_t mode)
//{
//	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
//
//	// The index pool instance must be accessible during node construction
//	auto indexpool = m_indexpool.lock();
//	if(indexpool == nullptr) throw LinuxException(LINUX_ENOENT);
//
//	// If this isn't a directory node, a new node cannot be created underneath it
//	if(m_type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//
//	// The name must be non-NULL when creating a child node object
//	if((name == nullptr) || (*name == 0)) throw LinuxException(LINUX_EINVAL);
//	size_t namelen = _tcslen(name);
//
//	// Force the mode to represent a directory object
//	mode = (mode & ~LINUX_S_IFMT) | LINUX_S_IFDIR;
//
//	// Determine the amount of space that needs to be allocated for the directory path name string; when 
//	// providing NULL for the output, this will include the count for the NULL terminator
//	size_t pathlen = GetFinalPathNameByHandle(m_handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
//	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());
//
//	// Retrieve the path to the directory object based on the handle
//	std::vector<tchar_t> buffer(pathlen + 1 + namelen);
//	pathlen = GetFinalPathNameByHandle(m_handle, buffer.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
//	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());
//
//	// Append a path separator character and copy in the new node name
//	buffer[pathlen] = _T('\\');
//	_tcsncat_s(buffer.data(), buffer.size(), name, namelen);
//
//	// TODO: SECURITY_ATTRIBUTES based on mode
//	if(!::CreateDirectory(buffer.data(), nullptr)) {
//		
//		DWORD result = GetLastError();						// Get the windows error that occurred
//
//		// The exception that can be thrown differs based on the result from ::CreateDirectory()
//		if(result == ERROR_ALREADY_EXISTS) throw LinuxException(LINUX_EEXIST, Win32Exception(result));
//		else if(result == ERROR_PATH_NOT_FOUND) throw LinuxException(LINUX_ENOENT, Win32Exception(result));
//		else throw LinuxException(LINUX_EINVAL, Win32Exception(result));
//	}
//
//	// The handle to the directory is not returned, it must be opened specifically for use
//	// by the Node instance ...
//	HANDLE handle = OpenHostDirectory(buffer.data());
//	try { return std::make_shared<Node>(indexpool, NodeType::Directory, handle); }
//	catch(...) { CloseHandle(handle); throw; }
//}

//FileSystem::NodePtr HostFileSystem::Node::CreateSymbolicLink(const tchar_t* name, const tchar_t* target)
//{
//	_ASSERTE(m_handle != INVALID_HANDLE_VALUE);
//
//	// The index pool instance must be accessible during node construction
//	auto indexpool = m_indexpool.lock();
//	if(indexpool == nullptr) throw LinuxException(LINUX_ENOENT);
//
//	// If this isn't a directory node, a new node cannot be created underneath it
//	if(m_type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//
//	// The name must be non-NULL when creating a child node object
//	if((name == nullptr) || (*name == 0)) throw LinuxException(LINUX_EINVAL);
//	size_t namelen = _tcslen(name);
//
//	// Determine the amount of space that needs to be allocated for the directory path name string; when 
//	// providing NULL for the output, this will include the count for the NULL terminator
//	size_t pathlen = GetFinalPathNameByHandle(m_handle, nullptr, 0, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
//	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());
//
//	// Retrieve the path to the directory object based on the handle
//	std::vector<tchar_t> buffer(pathlen + 1 + namelen);
//	pathlen = GetFinalPathNameByHandle(m_handle, buffer.data(), pathlen, FILE_NAME_NORMALIZED | VOLUME_NAME_DOS);
//	if(pathlen == 0) throw LinuxException(LINUX_EINVAL, Win32Exception());
//
//	// Append a path separator character and copy in the new node name
//	buffer[pathlen] = _T('\\');
//	_tcsncat_s(buffer.data(), buffer.size(), name, namelen);
//
//	// TODO: adjust target for host
//	// todo: flags, 0 = file
//	// todo: exception codes
//	if(!::CreateSymbolicLink(buffer.data(), target, 0)) {
//		
//		DWORD dw = GetLastError();
//		// 1314 = required privilege not held
//		throw LinuxException(LINUX_EINVAL, Win32Exception());
//	}
//
//	return nullptr;
//	// todo
//	//// The handle to the directory is not returned, it must be opened specifically for use
//	//// by the Node instance ...
//	//HANDLE handle = OpenHostDirectory(buffer.data());
//	//try { return std::make_shared<Node>(indexpool, NodeType::Directory, handle); }
//	//catch(...) { CloseHandle(handle); throw; }
//}

//-----------------------------------------------------------------------------

#pragma warning(pop)
