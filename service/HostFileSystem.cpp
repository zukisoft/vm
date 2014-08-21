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
//	root		- Pointer to the root node for the file system

HostFileSystem::HostFileSystem(const std::shared_ptr<Node>& root) : m_root(root)
{
	_ASSERTE(root);
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
	std::shared_ptr<Node> root = NodeFromPath(device);
	if(root->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	// todo: this will need volume and quota information eventually when the
	// superblock-style functions are written
	return std::make_shared<HostFileSystem>(root);
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
	//
	// Will also need to verify that when things are opened up from the file
	// system that they are contained in the mount point, so perhaps I need to
	// pass the HostFileSystem instance along after all, which would allow it
	// to hold code to deal with the node numbers again too.  hmmmmmmm

	(name);
	(target);
	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
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
	return HostFileSystem::NodeFromPath(AppendToPath(path));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
