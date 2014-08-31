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
#include "TempFileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TempFileSystem Constructor
//
// Arguments:
//
//	mountpoint	- Reference to the mountpoint instance for this file system

TempFileSystem::TempFileSystem(const std::shared_ptr<MountPoint>& mountpoint, const std::shared_ptr<Alias>& alias) : 
	m_mountpoint(mountpoint), m_root(alias)
{
	_ASSERTE(mountpoint);
	_ASSERTE(alias);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Mount (static)
//
// Mounts the temporary file system
//
// Arguments:
//
//	source		- Unused for TempFileSystem
//	flags		- Standard mounting flags and attributes
//	data		- Additional file-system specific mounting options

FileSystemPtr TempFileSystem::Mount(const tchar_t*, uint32_t flags, void* data)
{
	// Create the shared MountPoint instance to be passed to all file system objects
	std::shared_ptr<MountPoint> mountpoint = std::make_shared<MountPoint>(flags, data);

	// Construct the TempFileSystem instance, providing an alias attached to a
	// new DirectoryNode instance that serves as the root node
	return std::make_shared<TempFileSystem>(mountpoint, Alias::Construct(_T(""), DirectoryNode::Construct(mountpoint)));
}

//
// TEMPFILESYSTEM::ALIAS
//

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::Constructor (private)
//
// Arguments:
//
//	name		- Name to assign to this Alias instance

TempFileSystem::Alias::Alias(const tchar_t* name, const std::shared_ptr<TempFileSystem::Node>& node) : m_name(name)
{
	_ASSERTE(name);
	_ASSERTE(node);

	// Due to the ability to overmount, the initial node is still "mounted" by
	// pushing it onto the stack; the difference is that it cannot be "unmounted"
	m_mounted.push(node);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::Construct (static)
//
// Constructs a new Alias instance
//
// Arguments:
//
//	name		- Name to assign to this Alias instance
//	node		- Initial Node instance to be assigned to this Alias

std::shared_ptr<TempFileSystem::Alias> TempFileSystem::Alias::Construct(const tchar_t* name, 
	const std::shared_ptr<TempFileSystem::Node>& node)
{
	// Construct a new shared Alias instance and return it to the caller
	return std::make_shared<Alias>(name, node);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::getNode (private)
//
// Accesses the topmost node referenced by this alias

FileSystem::NodePtr TempFileSystem::Alias::getNode(void) 
{ 
	std::lock_guard<std::mutex> critsec(m_lock);
	return m_mounted.empty() ? nullptr : m_mounted.top();
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::Mount (private)
//
// Mounts/binds a foreign node to this alias, obscuring the previous node
//
// Arguments:
//
//	node		- Foreign node to be mounted on this alias

void TempFileSystem::Alias::Mount(const FileSystem::NodePtr& node)
{
	_ASSERTE(node);

	// All that needs to be done for this file system is push the node
	std::lock_guard<std::mutex> critsec(m_lock);
	m_mounted.push(node);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::Unmount (private)
//
// Unmounts/unbinds a node from this alias, revealing the previously bound node
//
// Arguments:
//
//	NONE

void TempFileSystem::Alias::Unmount(void)
{
	// Pop the topmost node instance from the stack if there is more than one
	// Node instance pushed into it, otherwise do nothing at all
	std::lock_guard<std::mutex> critsec(m_lock);
	if(m_mounted.size() > 1) m_mounted.pop();
}

//
// TEMPFILESYSTEM::MOUNTPOINT
//

//-----------------------------------------------------------------------------
// TempFileSystem::MountPoint Constructor
//
// Arguments:
//
//	flags		- Standard mounting flags passed to Mount()
//	data		- Addtional custom mounting information for this file system

TempFileSystem::MountPoint::MountPoint(uint32_t flags, const void* data) : 
	m_options(flags, data), m_nextindex(FileSystem::NODE_INDEX_FIRSTDYNAMIC)
{
}

//
// TEMPFILESYSTEM::NODE
//

//-----------------------------------------------------------------------------
// TempFileSystem::Node::Constructor (private)
//
// Arguments:
//
//	mountpoint	- Reference to the parent filesystem's MountPoint instance
//	type		- Type of the Node being constructed

TempFileSystem::Node::Node(const std::shared_ptr<MountPoint>& mountpoint, FileSystem::NodeType type) : 
	m_mountpoint(mountpoint), m_index(mountpoint->AllocateIndex()), m_type(type)
{
	_ASSERTE(mountpoint);
}

//
// TEMPFILESYSTEM::DIRECTORYNODE
//

//-----------------------------------------------------------------------------
// TempFileSystem::DirectoryNode::Construct (static)
//
// Constructs a new DirectoryNode instance
//
// Arguments:
//
//	mountpoint	- Reference to the parent filesystem's MountPoint instance

std::shared_ptr<TempFileSystem::DirectoryNode> TempFileSystem::DirectoryNode::Construct(const std::shared_ptr<MountPoint>& mountpoint)
{
	// Construct a new shared DirectoryNode instance and return it to the caller
	return std::make_shared<DirectoryNode>(mountpoint);
}

//-----------------------------------------------------------------------------
// TempFileSystem::DirectoryNode::OpenHandle (private)
//
// Opens a Handle instance against this node
//
// Arguments:
//
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::DirectoryNode::OpenHandle(int flags)
{
	// Directory node handles must be opened in read-only mode
	if((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY) throw LinuxException(LINUX_EISDIR);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//
// TEMPFILESYSTEM::FILENODE
//

//-----------------------------------------------------------------------------
// TempFileSystem::FileNode::Construct (static)
//
// Constructs a new FileNode instance
//
// Arguments:
//
//	mountpoint	- Reference to the parent filesystem's MountPoint instance

std::shared_ptr<TempFileSystem::FileNode> TempFileSystem::FileNode::Construct(const std::shared_ptr<MountPoint>& mountpoint)
{
	// Construct a new shared DirectoryNode instance and return it to the caller
	return std::make_shared<FileNode>(mountpoint);
}

//-----------------------------------------------------------------------------
// TempFileSystem::FileNode::OpenHandle (private)
//
// Opens a Handle instance against this node
//
// Arguments:
//
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::FileNode::OpenHandle(int flags)
{
	// O_DIRECTORY verifies that the target node is a directory, which this is not
	if(flags & LINUX_O_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);

	// If the file system was mounted as read-only, write access cannot be granted
	if(m_mountpoint->Options.ReadOnly && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) throw LinuxException(LINUX_EROFS);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//
// TEMPFILESYSTEM::SYMBOLICLINKNODE
//

//-----------------------------------------------------------------------------
// TempFileSystem::SymbolicLinkNode::Construct (static)
//
// Constructs a new SymbolicLinkNode instance
//
// Arguments:
//
//	mountpoint	- Reference to the parent filesystem's MountPoint instance

std::shared_ptr<TempFileSystem::SymbolicLinkNode> TempFileSystem::SymbolicLinkNode::Construct(const std::shared_ptr<MountPoint>& mountpoint)
{
	// Construct a new shared DirectoryNode instance and return it to the caller
	return std::make_shared<SymbolicLinkNode>(mountpoint);
}

//-----------------------------------------------------------------------------
// TempFileSystem::SymbolicLinkNode::OpenHandle (private)
//
// Opens a Handle instance against this node
//
// Arguments:
//
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::SymbolicLinkNode::OpenHandle(int flags)
{
	// O_DIRECTORY?

	// Symbolic Links cannot be opened with O_NOFOLLOW unless O_PATH is also set
	if((flags & LINUX_O_NOFOLLOW) && ((flags & LINUX_O_PATH) == 0)) throw LinuxException(LINUX_ELOOP);

	// If the file system was mounted as read-only, write access cannot be granted
	if(m_mountpoint->Options.ReadOnly && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) throw LinuxException(LINUX_EROFS);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
