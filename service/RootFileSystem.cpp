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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// RootFileSystem Constructor (private)
//
// Arguments:
//
//	source		- Source/device name to use for the file system
//	flags		- File system flags and options
//	root		- Reference to the constructed root directory node

RootFileSystem::RootFileSystem(const char_t* source, uint32_t flags, const std::shared_ptr<Directory>& root) : 
	m_source(source), m_root(root)
{
	_ASSERTE(source);
	_ASSERTE(root);

	// Initialize the statistics for this file system.  The file system itself will always
	// report a zero size with zero available as there is only the single directory node
	memset(&m_stats, 0, sizeof(uapi::statfs));

	m_stats.f_type		= LINUX_TMPFS_MAGIC;
	m_stats.f_bsize		= SystemInformation::PageSize;
	m_stats.f_files		= 1;
	m_stats.f_fsid		= FileSystem::GenerateFileSystemId();
	m_stats.f_namelen	= 255;
	m_stats.f_flags		= flags;
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

	Capabilities::Demand(Capability::SystemAdmin);

	// Default mode, uid and gid for the root directory node
	uapi::mode_t mode	= LINUX_S_IRWXU | LINUX_S_IRWXG | LINUX_S_IROTH | LINUX_S_IXOTH;	// 0775
	uapi::pid_t uid		= 0;
	uapi::pid_t gid		= 0;

	// Parse the provided mounting options
	// TODO: FILTER JUST THE ACCEPTED FLAGS HERE (SO FAR RDONLY AND KERNMOUNT)
	MountOptions options(flags, data, datalength);

	// Divide the mounting flags into overall file system and per-mount flags
	auto fsflags = options.Flags & ~LINUX_MS_PERMOUNT_MASK;
	auto mountflags = options.Flags & LINUX_MS_PERMOUNT_MASK;

	try {

		// mode=
		//
		// Sets the permission flags to apply to the root directory
		if(options.Arguments.Contains("mode")) mode = std::stoul(options.Arguments["mode"], 0, 0);

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

	// Construct the file system instance, using a new Directory as the root node
	auto fs = std::make_shared<RootFileSystem>(source, fsflags, Directory::Create(mode, uid, gid));

	// Construct and return the mount instance
	return Mount::Create(fs, mountflags);
}

//
// ROOTFILESYSTEM::DIRECTORY
//

//-----------------------------------------------------------------------------
// RootFileSystem::Directory Constructor (private)
//
// Arguments:
//
//	mode		- Permission flags to assign to the directory
//	uid			- User ID of the directory owner
//	gid			- Group ID of the directory owner

RootFileSystem::Directory::Directory(uapi::mode_t mode, uapi::pid_t uid, uapi::pid_t gid)
{
	FILETIME		creationtime;			// Time this node was constructed

	// The time at which this node was constructed in memory will serve as the 
	// initial access/creation/modification time stamp for the node
	GetSystemTimeAsFileTime(&creationtime);

	// Initialize the statistics for this node, much of which is static
	memset(&m_stats, 0, sizeof(uapi::stat));

	m_stats.st_dev		= (0 << 16) | 0;	// TODO: DEVICE ID; MAJOR WILL BE ZERO MINOR SHOULD AUTO-INCREMENT
	m_stats.st_ino		= 2;				// Always inode index 2
	m_stats.st_nlink	= 2;				// Always 2 subdirectories, "." and ".."
	m_stats.st_mode		= mode;
	m_stats.st_uid		= uid;
	m_stats.st_gid		= gid;
	m_stats.st_blksize	= SystemInformation::PageSize;

	uapi::FILETIMEToTimeSpec(creationtime, &m_stats.st_atime, &m_stats.st_atime_nsec);
	uapi::FILETIMEToTimeSpec(creationtime, &m_stats.st_mtime, &m_stats.st_mtime_nsec);
	uapi::FILETIMEToTimeSpec(creationtime, &m_stats.st_ctime, &m_stats.st_ctime_nsec);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::Create (static)
//
// Constructs a new Directory instance
//
// Arguments:
//
//	mode		- Node permission flags
//	uid			- User id of the directory owner
//	gid			- Group id of the directory owner

std::shared_ptr<RootFileSystem::Directory> RootFileSystem::Directory::Create(uapi::mode_t mode, uapi::pid_t uid, uapi::pid_t gid)
{
	// Force the mode_t flags to indicate this is a directory node
	return std::make_shared<Directory>((mode & ~LINUX_S_IFMT) | LINUX_S_IFDIR, uid, gid);
}

//-----------------------------------------------------------------------------
// RootFileSystem::CreateCharacterDevice (private)
//
// Creates a new character device node within the file system
//
// Arguments:
//
//	thispath	- Path that was used to resolve this node instance
//	name		- Name to assign to the newly created alias
//	mode		- Mode flags to assign to the newly created node
//	device		- Major and minor numbers of the newly created character device

void RootFileSystem::Directory::CreateCharacterDevice(const std::unique_ptr<FileSystem::Path>& thispath, const char_t* name, 
	uapi::mode_t mode, uapi::dev_t device)
{
	UNREFERENCED_PARAMETER(thispath);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(mode);
	UNREFERENCED_PARAMETER(device);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::CreateDirectory (private)
//
// Creates a new directory node within the file system
//
// Arguments:
//
//	thispath	- Path that was used to resolve this node instance
//	name		- Name to assign to the newly created alias
//	mode		- Permission flags to assign to the newly created node

void RootFileSystem::Directory::CreateDirectory(const std::unique_ptr<FileSystem::Path>& thispath, const char_t* name, uapi::mode_t mode)
{
	UNREFERENCED_PARAMETER(thispath);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(mode);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::CreateFile (private)
//
// Creates a new regular file node within the file system
//
// Arguments:
//
//	thispath	- Path that was used to resolve this node instance
//	name		- Name to assign to the newly created alias
//	flags		- File node access, creation and status flags
//	mode		- Mode flags to assign to the newly created node

std::shared_ptr<FileSystem::Handle> RootFileSystem::Directory::CreateFile(const std::unique_ptr<FileSystem::Path>& thispath, 
	const char_t* name, int flags, uapi::mode_t mode)
{
	UNREFERENCED_PARAMETER(thispath);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(mode);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::CreateSymbolicLink (private)
//
// Creates a new symbolic link node within the file system
//
// Arguments:
//
//	thispath	- Path that was used to resolve this node instance
//	name		- Name to assign to the newly created alias
//	target		- Path to the symbolic link target alias

void RootFileSystem::Directory::CreateSymbolicLink(const std::unique_ptr<FileSystem::Path>& thispath, const char_t* name, const char_t* target)
{
	UNREFERENCED_PARAMETER(thispath);
	UNREFERENCED_PARAMETER(name);
	UNREFERENCED_PARAMETER(target);

	throw LinuxException(LINUX_EPERM, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::DemandPermission
//
// Demands read/write/execute permissions for the node (MAY_READ, MAY_WRITE, MAY_EXECUTE)
//
// Arguments:
//
//	permission		- FileSystem::Permission bitmask to demand

void RootFileSystem::Directory::DemandPermission(FileSystem::Permission permission)
{
	// Do a dirty read (non-concurrent) on the member stats to build a permission set

	// todo how does the mount flags affect this (don't do it here)
	// should build a new permission class that takes in capabilities, mountflags, etc.

	// capabilities should be renamed, will include UID and GID as well, it's more
	// like a security context

	// this needs to be based on the node mode, the current uid/gid and the
	// current capabilities
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::Lookup
//
// Resolves a file system path using this node as the starting point
//
// Arguments:
//
//	thispath	- Path that was used to resolve this node instance
//	rootpath	- Root path to use when resolving a leading /
//	pathname	- Path to be resolved
//	flags		- Path resolution flags
//	reparses	- The current number of reparse points (symlinks) encountered

std::unique_ptr<FileSystem::Path> RootFileSystem::Directory::Lookup(const std::unique_ptr<Path>& thispath, const std::unique_ptr<Path>& rootpath, 
	const char_t* pathname, int flags, int* reparses)
{
	if((pathname == nullptr) || (reparses == nullptr)) throw LinuxException(LINUX_EFAULT);

	// The calling user must have EXECUTE permission on this node for a lookup to succeed
	DemandPermission(FileSystem::Permission::Execute);

	auto mountflags = thispath->Mount->Flags;

	// todo
	// do not update atime? this doesn't technically 'read' from the node, consider
	// if data needed to be looked up from disk if it were a real block device

	// can't do it this way, need to check for . and .., which are legitimate
	// if ..; return thsipath->Alias->parent
	// if . or zero length, return thispath->Duplicate

	if(*pathname) throw LinuxException(LINUX_ENOENT); 
	
	return thispath->Duplicate();
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::Open
//
// Creates a Handle instance against this node
//
// Arguments:
//
//	thispath	- Path that was used to resolve this node instance
//	flags		- Flags to apply to the created handle instance

std::shared_ptr<FileSystem::Handle> RootFileSystem::Directory::Open(const std::unique_ptr<Path>& thispath, int flags)
{
	// O_PATH is a special case that generates a FileSystem::PathHandle instead
	if(flags & LINUX_O_PATH) return OpenPath(thispath, flags);

	// Directory node handles must awlays be opened in read-only mode
	if((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY) throw LinuxException(LINUX_EISDIR);

	// Handle operations against a directory require READ access to the node
	if((flags & LINUX_O_PATH) == 0) DemandPermission(FileSystem::Permission::Read);

	return DirectoryHandle::Create(thispath, shared_from_this());
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::OpenPath (private)
//
// Creates a PathHandle instance against this node
//
// Arguments:
//
//	thispath	- Path that was used to resolve this node instance
//	flags		- Flags to apply to the created handle instance

std::shared_ptr<FileSystem::Handle> RootFileSystem::Directory::OpenPath(const std::unique_ptr<Path>& thispath, int flags)
{
	// O_PATH operations ignore all flags except O_ACCMODE, O_CLOEXEC, O_DIRECTORY, O_NOFOLLOW and O_PATH
	if(flags & LINUX_O_PATH) flags &= (LINUX_O_ACCMODE | LINUX_O_CLOEXEC | LINUX_O_DIRECTORY | LINUX_O_NOFOLLOW | LINUX_O_PATH);

	// Directory node handles must awlays be opened in read-only mode
	if((flags & LINUX_O_ACCMODE) != LINUX_O_RDONLY) throw LinuxException(LINUX_EISDIR);

	return FileSystem::PathHandle::Create(DirectoryHandle::Create(thispath, shared_from_this()));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::Stat
//
// Provides statistical information about this node
//
// Arguments:
//
//	stats		- Structure to receive the node statistics

void RootFileSystem::Directory::Stat(uapi::stat* stats)
{
	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	Concurrency::critical_section::scoped_lock cs(m_statslock);
	memcpy(stats, &m_stats, sizeof(uapi::stat));
}

//-----------------------------------------------------------------------------
// RootFileSystem::Directory::getType
//
// Gets the type of node represented by this object

FileSystem::NodeType RootFileSystem::Directory::getType(void)
{
	return FileSystem::NodeType::Directory;
}

//
// ROOTFILESYSTEM::DIRECTORYHANDLE
//

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle Constructor (private)
//
// Arguments:
//
//	path		- Reference to the Path that was used to locate the node
//	node		- Reference to the underlying Directory node instance

RootFileSystem::DirectoryHandle::DirectoryHandle(const std::unique_ptr<FileSystem::Path>& path, const std::shared_ptr<RootFileSystem::Directory>& node)
	: m_path(path->Duplicate()), m_node(node)
{
	_ASSERTE(path);
	_ASSERTE(node);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Create (static)
//
// Creates a new ExecuteHandle instance, wrapping an existing file system handle
//
// Arguments:
//
//	path		- Reference to the Path that was used to locate the node
//	node		- Reference to the underlying Directory node instance

std::shared_ptr<RootFileSystem::DirectoryHandle> RootFileSystem::DirectoryHandle::Create(const std::unique_ptr<FileSystem::Path>& path, 
	const std::shared_ptr<Directory>& node)
{
	return std::make_shared<DirectoryHandle>(path, node);
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Duplicate
//
// Creates a duplicate Handle instance
//
// Arguments:
//
//	flags		- Flags applicable to the new handle

std::shared_ptr<FileSystem::Handle> RootFileSystem::DirectoryHandle::Duplicate(int flags)
{
	return m_node->Open(m_path, flags);
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

	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
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

	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
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

	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void RootFileSystem::DirectoryHandle::Sync(void)
{
	// do nothing
}

//-----------------------------------------------------------------------------
// RootFileSystem::DirectoryHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void RootFileSystem::DirectoryHandle::SyncData(void)
{
	// do nothing
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
	
	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
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

	throw LinuxException(LINUX_EISDIR, Exception(E_NOTIMPL));
}

//
// ROOTFILESYSTEM::MOUNT
//

//-----------------------------------------------------------------------------
// RootFileSystem::Mount Constructor (private)
//
// Arguments:
//
//	fs		- Reference to the RootFileSystem instance
//	flags	- Per-mount flags and options to set on this mount instance

RootFileSystem::Mount::Mount(const std::shared_ptr<RootFileSystem>& fs, uint32_t flags) : m_fs(fs), m_flags(flags)
{
	_ASSERTE(fs);
}

//-------------------------------------------------------------------------------
// RootFileSystem::Mount::Create (static)
//
// Constructs a new Mount instance
//
// Arguments:
//
//	fs		- Reference to the RootFileSystem instance
//	flags	- Per-mount flags and options to set on this mount instance

std::shared_ptr<class RootFileSystem::Mount> RootFileSystem::Mount::Create(const std::shared_ptr<RootFileSystem>& fs, uint32_t flags)
{
	// Ensure that any file system specific flags are stripped from the mount flags
	return std::make_shared<Mount>(fs, flags & LINUX_MS_PERMOUNT_MASK);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Duplicate
//
// Duplicates this mount instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Mount> RootFileSystem::Mount::Duplicate(void)
{
	// Clone the underlying file system reference and flags into a new mount
	return std::make_shared<Mount>(m_fs, m_flags);
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getFlags
//
// Gets the flags set on this mount, includes file system flags

uint32_t RootFileSystem::Mount::getFlags(void)
{
	Concurrency::critical_section::scoped_lock cs(m_fs->m_statslock);

	// The flags are a combination of the file system flags and the per-mount flags,
	// this allows a caller to check options and permissions against the mount
	return static_cast<uint32_t>(m_fs->m_stats.f_flags) | m_flags;
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
	Capabilities::Demand(Capability::SystemAdmin);

	// MS_REMOUNT must be specified in the flags when calling this function
	if((flags & LINUX_MS_REMOUNT) != LINUX_MS_REMOUNT) throw LinuxException(LINUX_EINVAL);

	// Parse the provided mounting options into remount flags and key/value pairs
	MountOptions options(flags & LINUX_MS_RMT_MASK, data, datalen);

	// Prevent concurrent changes to the file system statistics
	Concurrency::critical_section::scoped_lock cs(m_fs->m_statslock);

	// Filter the flags to only those options which have changed from the current ones
	uint32_t changedflags = static_cast<uint32_t>(m_fs->m_stats.f_flags) ^ options.Flags;

	// TODO: do these flags need to be checked for validity, or are unsupported options
	// just generally ignored?  

	// NOTE: It's not possible to open a handle for write access in the root file system,
	// therefore there is no need to check the existing handles before changing MS_RDONLY

	// MS_RDONLY
	//
	if(changedflags & LINUX_MS_RDONLY)
		m_fs->m_stats.f_flags = (m_fs->m_stats.f_flags & ~LINUX_MS_RDONLY) | options[LINUX_MS_RDONLY];
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getRoot
//
// Gets a reference to the root node of the mount point

std::shared_ptr<FileSystem::Node> RootFileSystem::Mount::getRoot(void)
{
	return m_fs->m_root;
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::getSource
//
// Gets the device/name used as the source of the file system

const char_t* RootFileSystem::Mount::getSource(void)
{
	return m_fs->m_source.c_str();
}

//-----------------------------------------------------------------------------
// RootFileSystem::Mount::Stat
//
// Provides statistical information about the mounted file system
//
// Arguments:
//
//	stats		- Structure to receieve the file system statistics

void RootFileSystem::Mount::Stat(uapi::statfs* stats)
{
	if(stats == nullptr) throw LinuxException(LINUX_EFAULT);

	// Prevent concurrent changes to the file system statistics
	Concurrency::critical_section::scoped_lock cs(m_fs->m_statslock);
	
	// Copy the file system statistics and apply the per-mount flags
	memcpy(stats, &m_fs->m_stats, sizeof(uapi::statfs));
	stats->f_flags |= m_flags;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
