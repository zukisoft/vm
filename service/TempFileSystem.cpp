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
//	parent		- Parent alias instance, or nullptr if this is a root node
//	node		- Node to assign to this Alias instance

TempFileSystem::Alias::Alias(const tchar_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node) 
	: m_name(name), m_parent(parent)
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
	const FileSystem::NodePtr& node)
{
	// This version of Construct should only be called when constructing a root 
	// alias for the file system, invoke the other version with a nullptr parent
	return Construct(name, nullptr, node);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::Construct (static)
//
// Constructs a new Alias instance
//
// Arguments:
//
//	name		- Name to assign to this Alias instance
//	parent		- Parent alias for this Alias instance, or nullptr if this is root
//	node		- Initial Node instance to be assigned to this Alias

std::shared_ptr<TempFileSystem::Alias> TempFileSystem::Alias::Construct(const tchar_t* name, 
	const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node)
{
	// Construct a new shared Alias instance and return it to the caller
	return std::make_shared<Alias>(name, parent, node);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::getNode
//
// Accesses the topmost node referenced by this alias

FileSystem::NodePtr TempFileSystem::Alias::getNode(void) 
{ 
	std::lock_guard<std::mutex> critsec(m_lock);
	return m_mounted.empty() ? nullptr : m_mounted.top();
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::getParent
//
// Accesses the parent alias for this alias instance

FileSystem::AliasPtr TempFileSystem::Alias::getParent(void)
{
	// The parent is stored as a weak reference that must be converted
	FileSystem::AliasPtr parent = m_parent.lock();

	if(parent) return parent;
	else throw LinuxException(LINUX_ENOENT);
}

//-----------------------------------------------------------------------------
// TempFileSystem::Alias::Mount
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
// TempFileSystem::Alias::Unmount
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
// TempFileSystem::DirectoryNode::CreateDirectory
//
// Creates a new directory node as a child of this node
//
// Arguments:
//
//	parent		- Parent Alias instance from the resolved path
//	name		- Name to assign to the new directory object

void TempFileSystem::DirectoryNode::CreateDirectory(const FileSystem::AliasPtr& parent, const tchar_t* name)
{
	// The file system cannot be mounted as read-only when constructing new objects
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Construct the new DirectoryNode instance
	auto node = DirectoryNode::Construct(m_mountpoint);

	// Attempt to construct and insert a new Alias instance with the specified name
	auto result = m_children.insert(std::make_pair(name, Alias::Construct(name, parent, node)));
	if(!result.second) throw LinuxException(LINUX_EEXIST);
}

//-----------------------------------------------------------------------------
// TempFileSystem::DirectoryNode::CreateFile
//
// Creates a new regular file node as a child of this node
//
// Arguments:
//
//	parent		- Parent Alias instance from the resolved path
//	name		- Name to assign to the new file object
//	flags		- File creation flags

FileSystem::HandlePtr TempFileSystem::DirectoryNode::CreateFile(const FileSystem::AliasPtr& parent, const tchar_t* name, int flags)
{
	// The file system cannot be mounted as read-only when constructing new objects
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Construct the new FileNode instance and atomically create an initial handle
	// prior to adding it to the collection of child nodes
	auto node = FileNode::Construct(m_mountpoint /*, name */);
	auto handle = node->Open(flags);

	// Attempt to construct and insert a new Alias instance with the specified name
	auto result = m_children.insert(std::make_pair(name, Alias::Construct(name, parent, node)));
	if(!result.second) throw LinuxException(LINUX_EEXIST);

	return handle;				// Return Handle instance generated above
}

//-----------------------------------------------------------------------------
// TempFileSystem::DirectoryNode::CreateSymbolicLink
//
// Creates a new symbolic link node as a child of this node
//
// Arguments:
//
//	parent		- Parent Alias instance from the resolved path
//	name		- Name to assign to the new symbolic link object
//	target		- Target string to assign to the new symbolic link object

void TempFileSystem::DirectoryNode::CreateSymbolicLink(const FileSystem::AliasPtr& parent, const tchar_t* name, const tchar_t* target)
{
	// The file system cannot be mounted as read-only when constructing new objects
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Construct the new SymbolicLinkNode instance
	auto node = SymbolicLinkNode::Construct(m_mountpoint, target);

	// Attempt to construct and insert a new Alias instance with the specified name
	auto result = m_children.insert(std::make_pair(name, Alias::Construct(name, parent, node)));
	if(!result.second) throw LinuxException(LINUX_EEXIST);
}

//-----------------------------------------------------------------------------
// TempFileSystem::DirectoryNode::OpenHandle
//
// Opens a Handle instance against this node
//
// Arguments:
//
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::DirectoryNode::Open(int flags)
{
	// Directory node handles must be opened in read-only mode
	if((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY) throw LinuxException(LINUX_EISDIR);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

// DirectoryNode::RemoveNode
//
void TempFileSystem::DirectoryNode::RemoveNode(const tchar_t* name)
{
	// todo:
	// m_permission.Demand(blah);

	// Locate the child alias in the collection, ENOENT if not found
	auto found = m_children.find(name);
	if(found == m_children.end()) throw LinuxException(LINUX_ENOENT);

	// The node type has to be checked
	FileSystem::NodePtr node = found->second->Node;
	if(node->Type == FileSystem::NodeType::Directory) throw LinuxException(LINUX_EISDIR);
	
	// todo: Remove from the collection, node will die of on it's own
}

// DirectoryNode::ResolvePath
//
FileSystem::AliasPtr TempFileSystem::DirectoryNode::Resolve(const FileSystem::AliasPtr& current, const tchar_t* path, ResolveState& state)
{
	// Path string should not be null
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);
	
	// Construct a PathIterator instance to assist with traversing the path components
	PathIterator iterator(path);

	// Move past any "." components in the path before checking if the end of the
	// traversal was reached; which indicates that this is the target node
	while(_tcscmp(iterator.Current, _T(".")) == 0) ++iterator;
	if(!iterator) return current;

	// The ".." component indicates that the parent alias' node needs to resolve the remainder
	if(_tcscmp(iterator.Current, _T("..")) == 0) 
		return current->Parent->Node->Resolve(current->Parent, iterator.Remaining, state);

	// Attempt to locate the next component in the child collection
	auto found = m_children.find(iterator.Current);
	if(found != m_children.end()) return found->second->Node->Resolve(found->second, iterator.Remaining, state);

	// Path component was not found
	throw LinuxException(LINUX_ENOENT);
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
// TempFileSystem::FileNode::OpenHandle
//
// Opens a Handle instance against this node
//
// Arguments:
//
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::FileNode::Open(int flags)
{
	// O_DIRECTORY verifies that the target node is a directory, which this is not
	if(flags & LINUX_O_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);

	// If the file system was mounted as read-only, write access cannot be granted
	if(m_mountpoint->Options.ReadOnly && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) throw LinuxException(LINUX_EROFS);

	// Demand the proper permission based on the access mode flags provided
	switch(flags & LINUX_O_ACCMODE) {

		case LINUX_O_RDONLY: m_permission.Demand(FilePermission::Access::Read); break;
		case LINUX_O_WRONLY: m_permission.Demand(FilePermission::Access::Write); break;
		case LINUX_O_RDWR:   m_permission.Demand(FilePermission::Access::Read | FilePermission::Access::Write); break;
	}

	// Create a new permission for the handle, which is narrowed from the node permission
	// based on the file access mask flags
	FilePermission permission(m_permission);
	permission.Narrow(flags);

	// O_TRUNC: Truncate the file when the handle is opened, although this requires
	// write access to succeed, it is not an exception to request it with read-only handles
	if((flags & LINUX_O_TRUNC) && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) {

		Concurrency::reader_writer_lock::scoped_lock writer(m_lock);
		m_data.clear();
	}

	// Construct and return the new Handle instance for this node
	return std::make_shared<Handle>(shared_from_this(), flags, permission);
}

// FileNode::ResolvePath
//
FileSystem::AliasPtr TempFileSystem::FileNode::Resolve(const FileSystem::AliasPtr& current, const tchar_t* path, FileSystem::ResolveState& state)
{
	bool isleaf = (path == nullptr) ? true : (*path == 0);

	if(state.DirectoryRequired) throw LinuxException(LINUX_ENOTDIR);

	// File nodes can only be resolved to themselves, they have no children
	if(!isleaf) throw LinuxException(LINUX_ENOTDIR);
	return current;
}

// ----------------------------------------------------------------------------
// TEMPFILESYSTEM::FILENODE::HANDLE IMPLEMENTATION
// ----------------------------------------------------------------------------

// FileNode::Handle Constructor
//
TempFileSystem::FileNode::Handle::Handle(const std::shared_ptr<FileNode>& node, int flags, const FilePermission& permission) :
	m_flags(flags), m_node(node), m_position(0), m_permission(permission) 
{
}

// FileNode::Handle::Read
//
uapi::size_t TempFileSystem::FileNode::Handle::Read(void* buffer, uapi::size_t count)
{
	if(!buffer) throw LinuxException(LINUX_EFAULT);

	// Demand read permissions for this file
	m_permission.Demand(FilePermission::Access::Read);

	// Acquire a reader lock against the file data buffer
	Concurrency::reader_writer_lock::scoped_lock_read reader(m_node->m_lock);
	
	// The current file position can be beyond the end from a Seek()
	size_t position = m_position;
	position = min(m_node->m_data.size(), position);

	// Determine the number of bytes to read from the file data
	count = min(count, m_node->m_data.size() - position);

	// Read the data from the file into the caller-supplied buffer
	if(count) memcpy(buffer, m_node->m_data.data() + position, count);

	m_position += count;			// Advance the file pointer
	return count;					// Return number of bytes read
}

// FileNode::Handle::Seek
//
uapi::loff_t TempFileSystem::FileNode::Handle::Seek(uapi::loff_t offset, int whence)
{
	// This is only necessary since the node data is accessed for SEEK_END
	Concurrency::reader_writer_lock::scoped_lock_read reader(m_node->m_lock);

	// Note that it's not an error to move the file pointer beyond the end of the file;
	// this must be accounted for with boundary checking in functions that use it
	switch(whence)
	{
		// SEEK_SET: The new offset is from the beginning of the file
		case LINUX_SEEK_SET: 
			break;

		// SEEK_CUR: The new offset is relative to the current position
		case LINUX_SEEK_CUR: 
			offset += m_position; 
			break;
		
		// SEEK_END: The new offset is relative to the end of the file
		case LINUX_SEEK_END:
			offset += m_node->m_data.size();
			break;
		
		default: throw LinuxException(LINUX_EINVAL);
	}

	// Ensure that the resultant file pointer isn't negative or too big (x86)
	if(offset < 0) throw LinuxException(LINUX_EINVAL);
	if(offset > MAXSIZE_T) throw LinuxException(LINUX_EFBIG);

	m_position = static_cast<size_t>(offset);	// Set the new position
	return m_position;							// Return the new position
}

// FileNode::Handle::Sync
//
void TempFileSystem::FileNode::Handle::Sync(void)
{
	// Demand write permission to the file but otherwise there is nothing
	// useful to do for TempFileSystem, there is no underlying storage
	m_permission.Demand(FilePermission::Access::Write);
}

// FileNode::Handle::SyncData
//
void TempFileSystem::FileNode::Handle::SyncData(void)
{
	// Demand write permission to the file but otherwise there is nothing
	// useful to do for TempFileSystem, there is no underlying storage
	m_permission.Demand(FilePermission::Access::Write);
}

// FileNode::Handle::Write
//
uapi::size_t TempFileSystem::FileNode::Handle::Write(const void* buffer, uapi::size_t count)
{
	if(!buffer) throw LinuxException(LINUX_EFAULT);

#ifndef _M_X64
	// 32-bit builds can only store data up to size_t in length.  In reality the
	// file couldn't even get close to this large in memory, but EFBIG should be
	// thrown rather than ENOSPC if this is known to be the case up front
	uint64_t finalsize = m_position;
	if(finalsize + count > MAXSIZE_T) throw LinuxException(LINUX_EFBIG);
#endif

	// Demand write permissions for this file
	m_permission.Demand(FilePermission::Access::Write);

	// Acquire a writer lock against the file data buffer
	Concurrency::reader_writer_lock::scoped_lock writer(m_node->m_lock);

	// O_APPEND: Move the file pointer to the end of file before writing
	if(m_flags & LINUX_O_APPEND) m_position = m_node->m_data.size();

	// Attempt to resize the vector<> large enough to hold the new data
	try { m_node->m_data.resize(max(m_node->m_data.size(), m_position + count)); }
	catch(std::bad_alloc&) { throw LinuxException(LINUX_ENOSPC); }

	// Copy the caller-supplied data into the file data buffer
	if(count) memcpy(m_node->m_data.data() + m_position, buffer, count);

	m_position += count;				// Advance the file pointer
	return count;						// Return number of bytes written
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
// TEMPFILESYSTEM::NODEBASE
//

//-----------------------------------------------------------------------------
// TempFileSystem::NodeBase::Constructor (private)
//
// Arguments:
//
//	mountpoint	- Reference to the parent filesystem's MountPoint instance
//	type		- Type of the Node being constructed

TempFileSystem::NodeBase::NodeBase(const std::shared_ptr<MountPoint>& mountpoint, FileSystem::NodeType type) : 
	m_mountpoint(mountpoint), m_index(mountpoint->AllocateIndex()), m_type(type), m_permission(0)
{
	_ASSERTE(mountpoint);
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
//	target		- Target string to assign to the symbolic link node

std::shared_ptr<TempFileSystem::SymbolicLinkNode> 
TempFileSystem::SymbolicLinkNode::Construct(const std::shared_ptr<MountPoint>& mountpoint, const tchar_t* target)
{
	// Target string cannot be null or empty 
	if(target == nullptr) throw LinuxException(LINUX_EFAULT);
	if(*target == 0) throw LinuxException(LINUX_ENOENT);

	// Construct a new shared DirectoryNode instance and return it to the caller
	return std::make_shared<SymbolicLinkNode>(mountpoint, target);
}

FileSystem::AliasPtr TempFileSystem::SymbolicLinkNode::Follow(const FileSystem::AliasPtr& current)
{
	//
	// JUST MOVE THIS BACK TO RESOLVE(), remove Follow() from SymbolicLink interface -- need state to call Resolve()
	//

	// Process the symbolic link by sending the target into the current alias' parent
	ResolveState state(FileSystem::ResolveFlags::None);  // <--- TODO: is this right??
	return current->Parent->Node->Resolve(current->Parent, m_target.c_str(), state);
}

uapi::size_t TempFileSystem::SymbolicLinkNode::ReadTarget(tchar_t* buffer, size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	// Copy the minimum of the target length or the output buffer size
	count = min(m_target.size(), count);
	memcpy(buffer, m_target.data(), count * sizeof(tchar_t));
	
	return count;
}

////-----------------------------------------------------------------------------
//// TempFileSystem::SymbolicLinkNode::CreateDirectory
////
//// Creates a new directory node as a child of this node
////
//// Arguments:
////
////	parent		- Parent Alias instance from the resolved path
////	name		- Name to assign to the new directory object
//
//void TempFileSystem::SymbolicLinkNode::CreateDirectory(const FileSystem::AliasPtr& parent, const tchar_t* name)
//{
//	// TODO: O_NOFOLLOW?
//
//	// Resolve the target using our parent in the Alias tree and ask that node to create the object
//	parent->Parent->Node->ResolvePath(parent->Parent, m_target.c_str())->Node->CreateDirectory(parent, name);
//}
//
////-----------------------------------------------------------------------------
//// TempFileSystem::SymbolicLinkNode::CreateFile
////
//// Creates a new regular file node as a child of this node
////
//// Arguments:
////
////	parent		- Parent Alias instance from the resolved path
////	name		- Name to assign to the new file object
////	flags		- File creation flags
//
//FileSystem::HandlePtr TempFileSystem::SymbolicLinkNode::CreateFile(const FileSystem::AliasPtr& parent, const tchar_t* name, int flags)
//{
//	// This operation cannot succeed against the symbolic link itself
//	if(flags & LINUX_O_NOFOLLOW) throw LinuxException(LINUX_ENOTDIR);
//
//	// Resolve the target using our parent in the Alias tree and ask that node to create the object
//	return parent->Parent->Node->ResolvePath(parent->Parent, m_target.c_str())->Node->CreateFile(parent, name, flags);
//}
//
////-----------------------------------------------------------------------------
//// TempFileSystem::SymbolicLinkNode::CreateSymbolicLink
////
//// Creates a new symbolic link node as a child of this node
////
//// Arguments:
////
////	parent		- Parent Alias instance from the resolved path
////	name		- Name to assign to the new symbolic link object
////	target		- Target string to assign to the new symbolic link object
//
//void TempFileSystem::SymbolicLinkNode::CreateSymbolicLink(const FileSystem::AliasPtr& parent, const tchar_t* name, const tchar_t* target)
//{
//	// TODO: O_NOFOLLOW?
//
//	// Resolve the target using our parent in the Alias tree and ask that node to create the object
//	parent->Parent->Node->ResolvePath(parent->Parent, m_target.c_str())->Node->CreateSymbolicLink(parent, name, target);
//}
//
//-----------------------------------------------------------------------------
// TempFileSystem::SymbolicLinkNode::OpenHandle
//
// Opens a Handle instance against this node
//
// Arguments:
//
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::SymbolicLinkNode::Open(int flags)
{
	// O_DIRECTORY?
	m_permission.Demand(FilePermission::Access::Read);

	// Symbolic Links cannot be opened with O_NOFOLLOW unless O_PATH is also set
	if((flags & LINUX_O_NOFOLLOW) && ((flags & LINUX_O_PATH) == 0)) throw LinuxException(LINUX_ELOOP);

	// If the file system was mounted as read-only, write access cannot be granted
	if(m_mountpoint->Options.ReadOnly && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) throw LinuxException(LINUX_EROFS);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

////-----------------------------------------------------------------------------
//// TempFileSystem::SymbolicLinkNode::ResolvePath
////
//// Resolves a path relative from this Node instance
////
//// Arguments:
////
////	current		- Current Alias instance that resolved to this node instance
////	path		- Relative path to be resolved
//
//FileSystem::AliasPtr TempFileSystem::SymbolicLinkNode::ResolvePath(const FileSystem::AliasPtr& current, const tchar_t* path)
//{
//	// TODO: watch for ELOOP here, need a link counter argument; this will infinite loop
//	// if the symbolic link ends up referring to itself
//
//	// TODO: O_NOFOLLOW would resolve to this node rather than following the link if path is ""
//
//	// Process the symbolic link by sending the target into the current alias' parent
//	return current->Parent->Node->ResolvePath(current->Parent, m_target.c_str())->Node->ResolvePath(current, path);
//}

// SymbolicLinkNode::ResolvePath
//
FileSystem::AliasPtr TempFileSystem::SymbolicLinkNode::Resolve(const FileSystem::AliasPtr& current, const tchar_t* path, FileSystem::ResolveState& state)
{
	bool isleaf = (path == nullptr) ? true : (*path == 0);

	// If this is the leaf of the path and it's not supposed to be followed, check the state flags
	// and return the current alias if this symlink object should be returned
	if(isleaf && !state.FollowLeaf) {

		if(state.DirectoryRequired) throw LinuxException(LINUX_ENOTDIR);
		return current;
	}

	// Not a leaf or should be followed, follow the link and continue resolution from there
	if(state.IncrementDepth() > FileSystem::MAXIMUM_PATH_SYMLINKS) throw LinuxException(LINUX_ELOOP);
	return Follow(current)->Node->Resolve(current, path, state);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
