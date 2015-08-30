//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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
#include "RootFileSystem.h"

#include "Capability.h"
#include "FilePermission.h"
#include "MountOptions.h"
#include "LinuxException.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem Constructor
//
// Arguments:
//
//	source		- Source/device name to use for the file system
//	flags		- File system flags and options

RootFileSystem::RootFileSystem(const char_t* source, uint32_t flags) : m_source(source), m_flags(flags), m_fsid(FileSystem::GenerateFileSystemId())
{
	// No mount-specific flags should be specified for the file system instance
	_ASSERTE((flags & LINUX_MS_PERMOUNT_MASK) == 0);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount (static)
//
// Creates an instance of the file system
//
// Arguments:
//
//	source		- Source device path (ignored)
//	flags		- Standard mount options bitmask
//	data		- Extended mount options data
//	datalength	- Length of the extended mount options data in bytes

std::shared_ptr<FileSystem::Mount> RootFileSystem::Mount(const char_t* source, uint32_t flags, const void* data, size_t datalength)
{
	if(source == nullptr) throw LinuxException(LINUX_EFAULT);

	Capability::Demand(Capability::SystemAdmin);

	// Default mode, uid and gid for the root directory node
	uapi::mode_t mode	= LINUX_S_IRWXU | LINUX_S_IRWXG | LINUX_S_IROTH | LINUX_S_IXOTH;	// 0775
	uapi::uid_t uid		= 0;
	uapi::gid_t gid		= 0;

	// Parse the provided mounting options
	MountOptions options(flags, data, datalength);

	// Break up the standard mounting options bitmask into file system and mount specific masks
	auto fsflags = options.Flags & (LINUX_MS_RDONLY | LINUX_MS_KERNMOUNT | LINUX_MS_STRICTATIME);
	auto mountflags = (options.Flags & LINUX_MS_PERMOUNT_MASK) | LINUX_MS_NOEXEC | LINUX_MS_NODEV | LINUX_MS_NOSUID;

	try {

		// mode=
		//
		// Sets the permission flags to apply to the root directory
		if(options.Arguments.Contains("mode")) mode = (std::stoul(options.Arguments["mode"], 0, 0) & LINUX_S_IRWXUGO);

		// uid=
		//
		// Sets the owner UID to apply to the root directory
		if(options.Arguments.Contains("uid")) uid = std::stoul(options.Arguments["uid"], 0, 0);

		// gid=
		//
		// Sets the owner GID to apply to the root directory
		if(options.Arguments.Contains("gid")) gid = std::stoul(options.Arguments["gid"], 0, 0);
	}

	catch(...) { throw LinuxException(LINUX_EINVAL); }

	// Construct the file system instance and the root directory node instance
	auto fs = std::make_shared<RootFileSystem>(source, fsflags);
	auto rootdir = std::make_shared<DirectoryNode>(fs, mode, uid, gid);

	// Construct and return the mount instance
	return std::make_shared<class Mount>(fs, rootdir, mountflags);
}

//
// ROOTFILESYSTEM::DIRECTORYHANDLE
//

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle Constructor
//
// Arguments:
//
//	fs			- Parent file system instance
//	access		- Handle access mode
//	flags		- Handle flags

RootFileSystem::DirectoryHandle::DirectoryHandle(std::shared_ptr<RootFileSystem> fs, FileSystem::HandleAccess access, FileSystem::HandleFlags flags)
	: m_fs(std::move(fs)), m_access(access), m_flags(flags)
{
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle Destructor

RootFileSystem::DirectoryHandle::~DirectoryHandle()
{
	// Remove this handle instance from the file system's tracking collection
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	m_fs->m_handles.erase(this);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::getAccess
//
// Gets the handle access mode

FileSystem::HandleAccess RootFileSystem::DirectoryHandle::getAccess(void) const
{
	return m_access;
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Duplicate
//
// Creates a duplicate Handle instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Handle> RootFileSystem::DirectoryHandle::Duplicate(void) const
{
	// Construct the new handle object with the same access and flags as this handle
	auto handle = std::make_shared<DirectoryHandle>(m_fs, m_access, m_flags);

	// Place a weak reference to the handle into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(!m_fs->m_handles.emplace(handle.get(), handle).second) throw LinuxException(LINUX_ENOMEM);

	return handle;
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::getFlags
//
// Gets the flags specified on the handle

FileSystem::HandleFlags RootFileSystem::DirectoryHandle::getFlags(void) const
{
	return m_flags;
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Destination buffer
//	count		- Maximum number of bytes to read

uapi::size_t RootFileSystem::DirectoryHandle::Read(void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::ReadAt
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	offset		- Absolute file position to begin reading from
//	buffer		- Destination buffer
//	count		- Maximum number of bytes to read

uapi::size_t RootFileSystem::DirectoryHandle::ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Seek
//
// Changes the file position
//
// Arguments:
//
//	offset		- Offset (relative to whence) to position the file pointer
//	whence		- Flag indicating the file position from which offset applies

uapi::loff_t RootFileSystem::DirectoryHandle::Seek(uapi::loff_t offset, int whence)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(whence);

	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void RootFileSystem::DirectoryHandle::Sync(void) const
{
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void RootFileSystem::DirectoryHandle::SyncData(void) const
{
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer		- Source buffer
//	count		- Maximum number of bytes to write

uapi::size_t RootFileSystem::DirectoryHandle::Write(const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);
	
	throw LinuxException(LINUX_EISDIR);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::WriteAt
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	offset		- Absolute file position to begin writing from
//	buffer		- Source buffer
//	count		- Maximum number of bytes to write

uapi::size_t RootFileSystem::DirectoryHandle::WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException(LINUX_EISDIR);
}

//
// ROOTFILESYSTEM::DIRECTORYNODE
//

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode Constructor
//
// Arguments:
//
//	fs			- Reference to the parent file system instance
//	mode		- Permission flags to assign to the directory
//	uid			- User ID of the directory owner
//	gid			- Group ID of the directory owner

RootFileSystem::DirectoryNode::DirectoryNode(std::shared_ptr<RootFileSystem> fs, uapi::mode_t mode, uapi::uid_t uid, uapi::gid_t gid) 
	: m_fs(std::move(fs)), m_uid(uid), m_gid(gid), m_ctime(datetime::now()), m_mtime(m_ctime), m_atime(m_ctime)
{
	// Force the mode flags to indicate that this is a directory object
	m_mode = (mode & ~LINUX_S_IFMT) | LINUX_S_IFDIR;
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::CreateDirectory
//
// Creates a new directory node within the file system
//
// Arguments:
//
//	mount		- Mount on which this directory was reached
//	name		- Name to assign to the new node
//	mode		- Mode to assign to the new node

std::shared_ptr<FileSystem::Alias> RootFileSystem::DirectoryNode::CreateDirectory(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode)
{
	UNREFERENCED_PARAMETER(mount);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(mode);

	if(mount->Flags & LINUX_MS_RDONLY) throw LinuxException(LINUX_EROFS);
	else throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::CreateFile
//
// Creates a new regular file node within the file system
//
// Arguments:
//
//	mount		- Mount on which this directory was reached
//	name		- Name to assign to the new node
//	mode		- Mode to assign to the new node

std::shared_ptr<FileSystem::Alias> RootFileSystem::DirectoryNode::CreateFile(std::shared_ptr<FileSystem::Mount> mount, const char_t* name, uapi::mode_t mode)
{
	UNREFERENCED_PARAMETER(mount);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(mode);

	if(mount->Flags & LINUX_MS_RDONLY) throw LinuxException(LINUX_EROFS);
	else throw LinuxException(LINUX_EPERM);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::Lookup
//
// Looks up the alias associated with a child of this node
//
// Arguments:
//
//	mount		- Mount on which this directory was reached
//	name		- Name of the child alias to look up

std::shared_ptr<FileSystem::Alias> RootFileSystem::DirectoryNode::Lookup(std::shared_ptr<FileSystem::Mount> mount, const char_t* name) const
{
	UNREFERENCED_PARAMETER(mount);
	UNREFERENCED_PARAMETER(name);

	sync::critical_section::scoped_lock cs{ m_cs };
	FilePermission::Demand(FilePermission::Execute, m_uid, m_gid, m_mode);

	throw LinuxException(LINUX_ENOENT);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::Open
//
// Creates a Handle instance against this node
//
// Arguments:
//
//	mount		- Mount on which this directory was reached
//	access		- Handle access mode
//	flags		- Handle flags

std::shared_ptr<FileSystem::Handle> RootFileSystem::DirectoryNode::Open(std::shared_ptr<FileSystem::Mount> mount, FileSystem::HandleAccess access, FileSystem::HandleFlags flags)
{
	_ASSERTE(std::dynamic_pointer_cast<class Mount>(mount));

	// Directory node handles must always be opened in read-only mode
	if(access != FileSystem::HandleAccess::ReadOnly) throw LinuxException(LINUX_EISDIR);

	// Check for flags that are incompatible with opening a directory file system object
	if(flags & (FileSystem::HandleFlags::Append | FileSystem::HandleFlags::Direct)) throw LinuxException(LINUX_EINVAL);

	// Read access to the directory node is required to open a handle against it
	sync::critical_section::scoped_lock cs{ m_cs };
	FilePermission::Demand(FilePermission::Read, m_uid, m_gid, m_mode);

	// Construct the DirectoryHandle instance that will be returned to the caller
	auto handle = std::make_shared<DirectoryHandle>(m_fs, access, flags);

	// Place a weak reference to the handle into the tracking collection before returning it
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(!m_fs->m_handles.emplace(handle.get(), handle).second) throw LinuxException(LINUX_ENOMEM);

	UpdateAccessTime(mount);

	return handle;
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::SetOwnership
//
// Changes the ownership of this node
//
// Arguments:
//
//	uid			- New ownership user identifier
//	gid			- New ownership group identifier

void RootFileSystem::DirectoryNode::SetOwnership(uapi::uid_t uid, uapi::gid_t gid)
{
	// todo: CAP_CHOWN - see chown(2), there is more to this

	sync::critical_section::scoped_lock cs{ m_cs };
	FilePermission::Demand(FilePermission::Write, m_uid, m_gid, m_mode);

	m_uid = uid;
	m_gid = gid;
	m_ctime = datetime::now();
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::SetPermissions
//
// Changes the permission flags for this node
//
// Arguments:
//
//	permissions		- New permission flags for the node

void RootFileSystem::DirectoryNode::SetPermissions(uapi::mode_t permissions)
{
	permissions &= ~LINUX_S_IFMT;		// Strip off non-permissions

	// todo: CAP_FSETID - see chmod(2), there is more to this
	// todo: CAP_FOWNER - see chmod(2), there is more to this

	sync::critical_section::scoped_lock cs{ m_cs };
	FilePermission::Demand(FilePermission::Write, m_uid, m_gid, m_mode);

	m_mode = ((m_mode & LINUX_S_IFMT) | permissions);
	m_ctime = datetime::now();
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::Stat
//
// Provides statistical information about this node
//
// Arguments:
//
//	stats		- Structure to receive the node statistics

void RootFileSystem::DirectoryNode::Stat(uapi::stat* stats) const
{
	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	sync::critical_section::scoped_lock cs{ m_cs };
	
	stats->st_dev		= (0 << 16) | 0;	// TODO: DEVICE ID; MAJOR WILL BE ZERO MINOR SHOULD AUTO-INCREMENT
	stats->st_ino		= 2;				// Always inode index 2
	stats->st_nlink		= 2;				// Always 2 subdirectories, "." and ".."
	stats->st_mode		= m_mode;
	stats->st_uid		= m_uid;
	stats->st_gid		= m_gid;
	stats->st_rdev		= (0 << 16) | 0;	// TODO
	stats->st_size		= 0;
	stats->st_blksize	= SystemInformation::PageSize;
	stats->st_blocks	= 0;
	stats->st_atime		= convert<uapi::timespec>(m_atime);
	stats->st_mtime		= convert<uapi::timespec>(m_mtime);
	stats->st_ctime		= convert<uapi::timespec>(m_ctime);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::getType
//
// Gets the type of node represented by this object

FileSystem::NodeType RootFileSystem::DirectoryNode::getType(void) const
{
	return FileSystem::NodeType::Directory;
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryNode::UpdateAccessTime (private)
//
// Updates the access time of the node
//
// Arguments:
//
//	mount		- Mount on which the directory node was reached

void RootFileSystem::DirectoryNode::UpdateAccessTime(std::shared_ptr<FileSystem::Mount> mount)
{
	// Read-only file systems should not update the access time
	if(mount->Flags & LINUX_MS_RDONLY) return;

	// If MS_STRICTATIME is set or MS_NOATIME/MS_NODIRATIME are not set, the access time gets updated
	if((mount->Flags & LINUX_MS_STRICTATIME) || ((mount->Flags & (LINUX_MS_NOATIME | LINUX_MS_NODIRATIME)) == 0)) {

		datetime now = datetime::now();
		sync::critical_section::scoped_lock critsec{ m_cs };

		// If MS_STRICTATIME is set, always update the last access time
		if(mount->Flags & LINUX_MS_STRICTATIME) m_atime = now;

		// MS_STRICTATIME is not set, only update if the previous atime is less than mtime, less than ctime,
		// or indicates a value that is more than one day in the past
		else if((m_atime < m_mtime) || (m_atime < m_ctime) || (m_atime < (now - timespan::days(1)))) m_atime = now;
	}
}

//
// ROOTFILESYSTEM::MOUNT
//

//-----------------------------------------------------------------------------
// RootFileSystem::Mount Constructor
//
// Arguments:
//
//	fs		- Reference to the RootFileSystem instance
//	root	- Root directory node instance
//	flags	- Per-mount flags and options to set on this mount instance

RootFileSystem::Mount::Mount(std::shared_ptr<RootFileSystem> fs, std::shared_ptr<DirectoryNode> root, uint32_t flags) 
	: m_fs(std::move(fs)), m_root(std::move(root)), m_flags(flags)
{
	// The flags should only contain bits from MS_PERMOUNT_MASK
	_ASSERTE((m_flags & ~LINUX_MS_PERMOUNT_MASK) == 0);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Duplicate
//
// Duplicates this mount instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Mount> RootFileSystem::Mount::Duplicate(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	// Clone the underlying file system reference and flags into a new mount
	return std::make_shared<Mount>(m_fs, root, m_flags);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getFlags
//
// Gets the flags set on this mount, includes file system flags

uint32_t RootFileSystem::Mount::getFlags(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	return m_flags | m_fs->m_flags;
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Remount
//
// Remounts the file system with different options
//
// Arguments:
//
//	flags		- Standard mounting option flags
//	data		- Extended/custom mounting options
//	datalength	- Length of the extended mounting options data

void RootFileSystem::Mount::Remount(uint32_t flags, const void* data, size_t datalen)
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	Capability::Demand(Capability::SystemAdmin);

	// MS_REMOUNT must be specified in the flags when calling this function
	if((flags & LINUX_MS_REMOUNT) != LINUX_MS_REMOUNT) throw LinuxException(LINUX_EINVAL);

	// Parse the provided mounting options into remount flags and key/value pairs
	MountOptions options(flags & LINUX_MS_RMT_MASK, data, datalen);

	// Filter the flags to only those options which have changed from the current ones
	uint32_t changedflags = (m_fs->m_flags & LINUX_MS_RMT_MASK) ^ options.Flags;

	// MS_RDONLY
	//
	// Note: all handles created by RootFileSystem are read-only by nature as they all
	// reference directories; there is no need to check them before changing MS_RDONLY
	if(changedflags & LINUX_MS_RDONLY)
		m_fs->m_flags = (m_fs->m_flags & ~LINUX_MS_RDONLY) | options[LINUX_MS_RDONLY];
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getRoot
//
// Gets a reference to the root directory of the mount point

std::shared_ptr<FileSystem::Directory> RootFileSystem::Mount::getRoot(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	return root;
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getSource
//
// Gets the device/name used as the source of the file system

std::string RootFileSystem::Mount::getSource(void) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	return std::string(m_fs->m_source);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Stat
//
// Provides statistical information about the mounted file system
//
// Arguments:
//
//	stats		- Structure to receieve the file system statistics

void RootFileSystem::Mount::Stat(uapi::statfs* stats) const
{
	auto root = m_root;
	if(!root) throw LinuxException(LINUX_ENODEV);

	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	stats->f_type		= LINUX_TMPFS_MAGIC;
	stats->f_bsize		= SystemInformation::PageSize;
	stats->f_blocks		= 0;
	stats->f_bfree		= 0;
	stats->f_bavail		= 0;
	stats->f_files		= 1;
	stats->f_ffree		= 0;
	stats->f_fsid		= m_fs->m_fsid;
	stats->f_namelen	= MAX_PATH;
	stats->f_frsize		= 512;
	stats->f_flags		= m_flags | m_fs->m_flags;
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Unmount
//
// Unmounts the file system
//
// Arguments:
//
//	NONE

void RootFileSystem::Mount::Unmount(void)
{
	// There can be no active handles opened against the file system
	sync::critical_section::scoped_lock critsec{ m_fs->m_cs };
	if(m_fs->m_handles.size() > 0) throw LinuxException(LINUX_EBUSY);

	// Ensure that the root directory node is also not still shared out
	if(m_root.use_count() > 1) throw LinuxException(LINUX_EBUSY);

	m_root.reset();			// Release the root directory node
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
