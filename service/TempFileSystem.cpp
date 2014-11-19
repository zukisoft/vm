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

FileSystemPtr TempFileSystem::Mount(const uapi::char_t*, uint32_t flags, const void* data)
{
	// Create the shared MountPoint instance to be passed to all file system objects
	std::shared_ptr<MountPoint> mountpoint = std::make_shared<MountPoint>(flags, data);

	// Construct the TempFileSystem instance, providing an alias attached to a
	// new DirectoryNode instance that serves as the root node
	return std::make_shared<TempFileSystem>(mountpoint, Alias::Construct("", DirectoryNode::Construct(mountpoint)));
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

TempFileSystem::Alias::Alias(const uapi::char_t* name, const FileSystem::AliasPtr& parent, const FileSystem::NodePtr& node) 
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

std::shared_ptr<TempFileSystem::Alias> TempFileSystem::Alias::Construct(const uapi::char_t* name, const FileSystem::NodePtr& node)
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

std::shared_ptr<TempFileSystem::Alias> TempFileSystem::Alias::Construct(const uapi::char_t* name, 
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

//-----------------------------------------------------------------------------
// TEMPFILESYSTEM::DIRECTORYNODE IMPLEMENTATION
//-----------------------------------------------------------------------------

// DirectoryNode::Construct (static)
//
std::shared_ptr<TempFileSystem::DirectoryNode> TempFileSystem::DirectoryNode::Construct(const std::shared_ptr<MountPoint>& mountpoint)
{
	// Construct a new shared DirectoryNode instance and return it to the caller
	return std::make_shared<DirectoryNode>(mountpoint);
}

// DirectoryNode::CreateDirectory
//
void TempFileSystem::DirectoryNode::CreateDirectory(const FileSystem::AliasPtr& parent, const uapi::char_t* name)
{
	// The file system cannot be mounted as read-only when constructing new objects
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Construct the new DirectoryNode instance
	auto node = DirectoryNode::Construct(m_mountpoint);

	// Attempt to construct and insert a new Alias instance with the specified name
	auto result = m_children.insert(std::make_pair(name, Alias::Construct(name, parent, node)));
	if(!result.second) throw LinuxException(LINUX_EEXIST);
}

// DirectoryNode::CreateFile
//
FileSystem::HandlePtr TempFileSystem::DirectoryNode::CreateFile(const FileSystem::AliasPtr& parent, const uapi::char_t* name, int flags)
{
	// The file system cannot be mounted as read-only when constructing new objects
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Construct the new FileNode instance and atomically create an initial handle
	// prior to adding it to the collection of child nodes
	auto node = FileNode::Construct(m_mountpoint /*, name */);
	auto alias = Alias::Construct(name, parent, node);
	auto handle = node->Open(alias, flags);

	// Attempt to construct and insert a new Alias instance with the specified name
	auto result = m_children.insert(std::make_pair(name, alias));
	if(!result.second) throw LinuxException(LINUX_EEXIST);

	return handle;				// Return Handle instance generated above
}

// DirectoryNode::CreateSymbolicLink
//
void TempFileSystem::DirectoryNode::CreateSymbolicLink(const FileSystem::AliasPtr& parent, const uapi::char_t* name, const uapi::char_t* target)
{
	// The file system cannot be mounted as read-only when constructing new objects
	if(m_mountpoint->Options.ReadOnly) throw LinuxException(LINUX_EROFS);

	// Construct the new SymbolicLinkNode instance
	auto node = SymbolicLinkNode::Construct(m_mountpoint, target);

	// Attempt to construct and insert a new Alias instance with the specified name
	auto result = m_children.insert(std::make_pair(name, Alias::Construct(name, parent, node)));
	if(!result.second) throw LinuxException(LINUX_EEXIST);
}

// DirectoryNode::Open
//
FileSystem::HandlePtr TempFileSystem::DirectoryNode::Open(const AliasPtr& alias, int flags)
{
	// Directory node handles must be opened in read-only mode
	if((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY) throw LinuxException(LINUX_EISDIR);

	// Create a new permission for the handle, which is narrowed from the node permission
	// based on the file access mask flags
	FilePermission permission(m_permission);
	permission.Narrow(flags);

	// Construct and return the new Handle instance for this node
	if(flags & LINUX_O_PATH) return std::make_shared<PathHandle>(shared_from_this(), alias, flags, permission);
	else return std::make_shared<Handle>(shared_from_this(), alias, flags, permission);
}

// DirectoryNode::RemoveNode
//
void TempFileSystem::DirectoryNode::RemoveNode(const uapi::char_t* name)
{
	// Write permission is required to remove a node from a directory
	m_permission.Demand(FilePermission::Access::Write);

	// Locate the child alias in the collection, ENOENT if not found
	auto found = m_children.find(name);
	if(found == m_children.end()) throw LinuxException(LINUX_ENOENT);

	// The node type has to be checked
	FileSystem::NodePtr node = found->second->Node;
	if(node->Type == FileSystem::NodeType::Directory) throw LinuxException(LINUX_EISDIR);
	
	// todo: Remove from the collection, node will die off on it's own
}

// DirectoryNode::ResolvePath
//
FileSystem::AliasPtr TempFileSystem::DirectoryNode::Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks)
{
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// Execute permission is required to traverse a directory node
	m_permission.Demand(FilePermission::Access::Execute);
	
	// Construct a PathIterator instance to assist with traversing the path components
	PathIterator iterator(path);

	// Move past any "." components in the path before checking if the end of the
	// traversal was reached; which indicates that this is the target node
	while(strcmp(iterator.Current, ".") == 0) ++iterator;
	if(!iterator) return current;

	// The ".." component indicates that the parent alias' node needs to resolve the remainder
	if(strcmp(iterator.Current, "..") == 0) 
		return current->Parent->Node->Resolve(root, current->Parent, iterator.Remaining, flags, symlinks);

	// Attempt to locate the next component in the child collection
	auto found = m_children.find(iterator.Current);
	if(found != m_children.end()) return found->second->Node->Resolve(root, found->second, iterator.Remaining, flags, symlinks);

	// Path component was not found
	throw LinuxException(LINUX_ENOENT);
}

//-----------------------------------------------------------------------------
// TEMPFILESYSTEM::DIRECTORYNODE::HANDLE IMPLEMENTATION
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// TEMPFILESYSTEM::FILENODE IMPLEMENTATION
//-----------------------------------------------------------------------------

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
// TempFileSystem::FileNode::Open
//
// Opens a Handle instance against this node
//
// Arguments:
//
//	alias		- Alias instance used to resolve this node
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::FileNode::Open(const AliasPtr& alias, int flags)
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
	if(flags & LINUX_O_PATH) return std::make_shared<PathHandle>(shared_from_this(), alias, flags, permission);
	else return std::make_shared<Handle>(shared_from_this(), alias, flags, permission);
}

//-----------------------------------------------------------------------------
// TempFileSystem::FileNode::OpenExec
//
// Opens a Handle instance against this node for the purposes of creating a
// new process object
//
// Arguments:
//
//	alias		- Alias (name) used when this node was resolved
//	flags		- Operational flags and attributes

FileSystem::HandlePtr TempFileSystem::FileNode::OpenExec(const std::shared_ptr<FileSystem::Alias>& alias, int flags)
{
	// TODO: This should really only be called by the Virtual Machine itself, not
	// sure exactly what flags and attributes to check for yet

	// If the file system was mounted with no-execute, no files can be used for a process
	if(m_mountpoint->Options.NoExecute) throw LinuxException(LINUX_EACCES);

	// Demand execute permissions for this object
	m_permission.Demand(FilePermission::Access::Execute);

	// TODO: for now only put read and execute on the handle permission,
	// may want to consider a special handle instance that doesn't demand Read
	FilePermission permission(LINUX_S_IRUSR | LINUX_S_IRGRP | LINUX_S_IROTH | LINUX_S_IXUSR | LINUX_S_IXGRP | LINUX_S_IXOTH);

	return std::make_shared<Handle>(shared_from_this(), alias, flags, permission);
}

// FileNode::Resolve
//
FileSystem::AliasPtr TempFileSystem::FileNode::Resolve(const AliasPtr&, const AliasPtr& current, const uapi::char_t* path, int flags, int*)
{
	if(path == nullptr) throw LinuxException(LINUX_ENOENT);

	// If the path operation required termination in a directory, it cannot end here
	if((flags & LINUX_O_DIRECTORY) == LINUX_O_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);

	// File nodes can only be resolved to themselves, they have no children
	if(*path != 0) throw LinuxException(LINUX_ENOTDIR);
	return current;
}

// ----------------------------------------------------------------------------
// TEMPFILESYSTEM::FILENODE::HANDLE IMPLEMENTATION
// ----------------------------------------------------------------------------

// FileNode::Handle::Read
//
uapi::size_t TempFileSystem::FileNode::Handle::Read(void* buffer, uapi::size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

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

TempFileSystem::NodeBase::NodeBase(const std::shared_ptr<MountPoint>& mountpoint) : 
	m_mountpoint(mountpoint), m_index(mountpoint->AllocateIndex()), m_permission(0)
{
	_ASSERTE(mountpoint);
}

//-----------------------------------------------------------------------------
// TempFileSystem::NodeBase::DemandPermission
//
// Demands read, write and/or execute permissions to the node
//
// Arguments:
//
//	mode		- Special MAY_READ, MAY_WRITE, MAY_EXECUTE mode flags from caller

void TempFileSystem::NodeBase::DemandPermission(uapi::mode_t mode)
{
	// A mode mask of zero is F_OK, and only determines that the node exists
	if((mode & LINUX_MAY_ACCESS) == 0) return;

	// Check MAY_WRITE against a read-only file system, this produces EROFS rather than EACCES
	if((mode & LINUX_MAY_WRITE) && (m_mountpoint->Options.ReadOnly)) throw LinuxException(LINUX_EROFS);

	// Demand the permission from the contained FilePermission object
	m_permission.Demand(static_cast<FilePermission::Access>(mode & LINUX_MAY_ACCESS));
}

//-----------------------------------------------------------------------------
// TEMPFILESYSTEM::SYMBOLICLINKNODE IMPLEMENTATION
//-----------------------------------------------------------------------------

// SymbolicLinkNode::Construct (static)
//
std::shared_ptr<TempFileSystem::SymbolicLinkNode> 
TempFileSystem::SymbolicLinkNode::Construct(const std::shared_ptr<MountPoint>& mountpoint, const uapi::char_t* target)
{
	// Target string cannot be null or empty 
	if(target == nullptr) throw LinuxException(LINUX_EFAULT);
	if(*target == 0) throw LinuxException(LINUX_ENOENT);

	// Construct a new shared SymbolicLinkNode instance and return it to the caller
	return std::make_shared<SymbolicLinkNode>(mountpoint, target);
}

// SymbolicLinkNode::Open
//
FileSystem::HandlePtr TempFileSystem::SymbolicLinkNode::Open(const AliasPtr& alias, int flags)
{
	//
	// THIS IS WHERE IT NEEDS TO REDIRECT TO THE TARGET AND INVOKE THAT NODE'S OPEN()
	//

	// Symbolic links can only be opened with O_NOFOLLOW and O_PATH set in the flags
	if((flags & (LINUX_O_NOFOLLOW | LINUX_O_PATH)) != (LINUX_O_NOFOLLOW | LINUX_O_PATH)) throw LinuxException(LINUX_ELOOP);

	// If the file system was mounted as read-only, write access cannot be granted
	if(m_mountpoint->Options.ReadOnly && ((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY)) throw LinuxException(LINUX_EROFS);
	
	// Construct and return the new Handle instance for this node, note that PathHandle
	// is never used in conjunction with symbolic links and that there is no need for
	// a permissions object as symlinks are always read/write/execute for all users
	return std::make_shared<Handle>(shared_from_this(), alias, flags);
}

// SymbolicLinkNode::ReadTarget
//
uapi::size_t TempFileSystem::SymbolicLinkNode::ReadTarget(uapi::char_t* buffer, size_t count)
{
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT);

	// Copy the minimum of the target length or the output buffer size
	count = min(m_target.size(), count);
	memcpy(buffer, m_target.data(), count * sizeof(uapi::char_t));
	
	return count;
}

// SymbolicLinkNode::ResolvePath
//
FileSystem::AliasPtr TempFileSystem::SymbolicLinkNode::Resolve(const AliasPtr& root, const AliasPtr& current, const uapi::char_t* path, int flags, int* symlinks)
{
	if((path == nullptr) || (symlinks == nullptr)) throw LinuxException(LINUX_EFAULT);

	// If this is the leaf of the path and it's not supposed to be followed, this is the target node
	if((*path == 0) && ((flags & LINUX_O_NOFOLLOW) == LINUX_O_NOFOLLOW)) {

		// Check for O_DIRECTORY flag, this node is not a directory object
		if((flags & LINUX_O_DIRECTORY) == LINUX_O_DIRECTORY) throw LinuxException(LINUX_ENOTDIR);
		return current;
	}

	// Increment the number of followed symbolic links and throw ELOOP if there are too many
	if(++(*symlinks) > FileSystem::MAXIMUM_PATH_SYMLINKS) throw LinuxException(LINUX_ELOOP);

	// Copy and check the first character of the target string to determine absolute v. relative
	auto node = (m_target.size() && (m_target[0] == '/')) ? root->Node : current->Parent->Node;

	// Trim off any leading slash characters to convert an absolute path into a relative one
	std::string relative(std::ltrim(m_target, '/'));

	// Follow the symbolic link to acquire the target Node instance
	auto followed = node->Resolve(root, current->Parent, relative.c_str(), flags, symlinks);

	// Construct an Alias to bridge between this symbolic link's parent and the target node
	auto bridge = Alias::Construct(current->Name, current->Parent, followed->Node);
	return bridge->Node->Resolve(root, bridge, path, flags, symlinks);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
