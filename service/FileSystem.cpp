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
#include "FileSystem.h"
#include "LinuxException.h"
#include "Namespace.h"
#include "Random.h"

#pragma warning(push, 4)

//
// FILESYSTEM::HANDLEACCESS
//

// FileSystem::HandleAccess::ReadOnly (static)
//
const FileSystem::HandleAccess FileSystem::HandleAccess::ReadOnly{ LINUX_O_RDONLY };

// FileSystem::HandleAccess::WriteOnly (static)
//
const FileSystem::HandleAccess FileSystem::HandleAccess::WriteOnly{ LINUX_O_WRONLY };

// FileSystem::HandleAccess::ReadWrite (static)
//
const FileSystem::HandleAccess FileSystem::HandleAccess::ReadWrite{ LINUX_O_RDWR };

//
// FILESYSTEM::HANDLEFLAGS
//

// FileSystem::HandleFlags::Append (static)
//
const FileSystem::HandleFlags FileSystem::HandleFlags::Append{ LINUX_O_APPEND };

// FileSystem::HandleFlags::DataSync (static)
//
const FileSystem::HandleFlags FileSystem::HandleFlags::DataSync{ LINUX_O_DSYNC };

// FileSystem::HandleFlags::Direct (static)
//
const FileSystem::HandleFlags FileSystem::HandleFlags::Direct{ LINUX_O_DIRECT };

// FileSystem::HandleFlags::NoAccessTime (static)
//
const FileSystem::HandleFlags FileSystem::HandleFlags::NoAccessTime{ LINUX_O_NOATIME };

// FileSystem::HandleFlags::None (static)
//
const FileSystem::HandleFlags FileSystem::HandleFlags::None{ 0 };

// FileSystem::HandleFlags::Sync (static)
//
const FileSystem::HandleFlags FileSystem::HandleFlags::Sync{ LINUX_O_SYNC };

// FileSystem::HandleFlags::Truncate (static)
//
const FileSystem::HandleFlags FileSystem::HandleFlags::Truncate{ LINUX_O_TRUNC };

//
// FILESYSTEM::LOOKUPFLAGS
//

// FileSystem::LookupFlags::Directory (static)
//
const FileSystem::LookupFlags FileSystem::LookupFlags::Directory{ LINUX_O_DIRECTORY };

// FileSystem::LookupFlags::NoFollow (static)
//
const FileSystem::LookupFlags FileSystem::LookupFlags::NoFollow{ LINUX_O_NOFOLLOW };

// FileSystem::LookupFlags::None (static)
//
const FileSystem::LookupFlags FileSystem::LookupFlags::None{ 0 };

////-----------------------------------------------------------------------------
//// FileSystem::CheckPermissions (static)
////
//// Demands read/write/execute permissions for a file system object
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be checked
////	flags		- Operation flags (AT_EACCESS, AT_SYMLINK_NO_FOLLOW)
////	mode		- Special MAY_READ, MAY_WRITE, MAY_EXECUTE flags
//
//void FileSystem::CheckPermissions(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
//	int flags, uapi::mode_t mode)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// Default behavior is to dereference symbolic links, sys_faccessat() can specify otherwise
//	int resolveflags = ((flags & LINUX_AT_SYMLINK_NOFOLLOW) == LINUX_AT_SYMLINK_NOFOLLOW) ? LINUX_O_NOFOLLOW : 0;
//
//	// TODO: support AT_EACCESS flag
//
//	// Demand the requested permission(s) from the target file system node
//	ResolvePath(root, base, path, resolveflags)->Node->DemandPermission(mode);
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::CreateCharacterDevice (static)
////
//// Creates a file system character device object
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be opened/created
////	mode		- Mode bitmask to use when creating the object
////	device		- Target device identifier
//
//void FileSystem::CreateCharacterDevice(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
//	uapi::mode_t mode, uapi::dev_t device)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// Split the path into branch and leaf components
//	PathSplitter splitter(path);
//
//	// Resolve the branch path to an Alias instance, must resolve to a Directory
//	auto branch = ResolvePath(root, base, splitter.Branch, 0);
//	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//
//	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
//	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);
//
//	// Request a new character device node be created as a child of the resolved directory
//	directory->CreateCharacterDevice(branch, splitter.Leaf, mode, device);
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::CreateDirectory (static)
////
//// Creates a file system directory object
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be opened/created
////	mode		- Mode bitmask to use if a new object is created
//
//void FileSystem::CreateDirectory(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
//	const char_t* path, uapi::mode_t mode)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// Split the path into branch and leaf components
//	PathSplitter splitter(path);
//
//	// Resolve the branch path to an Alias instance, must resolve to a Directory
//	auto branch = ResolvePath(root, base, splitter.Branch, 0);
//	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//	
//	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
//	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);
//
//	directory->CreateDirectory(branch, splitter.Leaf, mode);
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::CreateFile (static)
////
//// Creates a regular file object and returns a Handle instance
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be opened/created
////	flags		- Flags indicating how the object should be opened
////	mode		- Mode bitmask to use if a new object is created
//
//std::shared_ptr<FileSystem::Handle> FileSystem::CreateFile(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
//	const char_t* path, int flags, uapi::mode_t mode)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// O_PATH and O_DIRECTORY cannot be used when creating a regular file object
//	if((flags & LINUX_O_PATH) || (flags & LINUX_O_DIRECTORY)) throw LinuxException(LINUX_EINVAL);
//
//	// Split the path into branch and leaf components
//	PathSplitter splitter(path);
//
//	// Resolve the branch path to an Alias instance, must resolve to a Directory
//	auto branch = ResolvePath(root, base, splitter.Branch, flags);
//	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//
//	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
//	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);
//
//	// Request a new regular file be created as a child of the resolved directory
//	return directory->CreateFile(branch, splitter.Leaf, flags, mode);
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::CreateSymbolicLink
////
//// Creates a file system symbolic link object
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be opened/created
////	target		- Symbolic link target
//
//void FileSystem::CreateSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
//	const char_t* target)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// Split the path into branch and leaf components
//	PathSplitter splitter(path);
//
//	// Resolve the branch path to an Alias instance, must resolve to a Directory
//	auto branch = ResolvePath(root, base, splitter.Branch, 0);
//	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//	
//	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
//	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);
//
//	directory->CreateSymbolicLink(branch, splitter.Leaf, target);
//}

//-----------------------------------------------------------------------------
// FileSystem::GenerateFileSystemId (static)
//
// Generates a unique file system identifier (fsid)
//
// Arguments:
//
//	NONE

uapi::fsid_t FileSystem::GenerateFileSystemId(void)
{
	// This may need to be fancier or guarantee uniqueness at some point,
	// but for the immediate need a pseudo-random fsid_t structure will do
	return Random::Generate<uapi::fsid_t>();
}

////-----------------------------------------------------------------------------
//// FileSystem::GetAbsolutePath (static)
////
//// Retrieves the absolute path for a provided relative path
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Relative path to be converted into an absolute path
//
//void FileSystem::GetAbsolutePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& alias, char_t* path, size_t pathlen)
//{
//	std::vector<std::string> pathvec;
//
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT);
//	if(pathlen == 0) throw LinuxException(LINUX_ERANGE);
//
//	// TODO: not sure what to do if the current directory has been deleted (unlinked)
//
//	// Start at the specified file system alias and continue working backwards until a root node is found
//	std::shared_ptr<Alias> current = alias;
//	while(current->Parent != current) {
//
//		// If the current node is a symbolic link, follow it to the target and loop again
//		if(current->Node->Type == NodeType::SymbolicLink) {
//
//			auto symlink = std::dynamic_pointer_cast<SymbolicLink>(current->Node);
//
//			// if this throws, should it be "(unreachable)" -- see kernel code
//			int loop = 0;	// For ELOOP detection
//			current = symlink->Resolve(root, current, nullptr, 0, &loop);
//			continue;
//		}
//
//		// Should never happen, but check for it regardless
//		if(current->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//
//		// Push the next Alias name into the path building collection and move up to parent
//		pathvec.push_back(current->Name);
//		current = current->Parent;
//	}
//
//	pathvec.push_back("");
//
//	std::string tododeleteme;
//	for (auto iterator = pathvec.begin(); iterator != pathvec.end(); iterator++) {
//			
//		tododeleteme += "/";
//		tododeleteme += *iterator;
//	}
//
//	if(tododeleteme.length() + 1 > pathlen) throw LinuxException(LINUX_ERANGE);
//	strncpy_s(path, pathlen, tododeleteme.c_str(), _TRUNCATE);
//}

//-----------------------------------------------------------------------------
// FileSystem::LookupPath (static)
//
// Resolves a path to a file system object
//
// Arguments:
//
//	ns			- Namespace associated with the calling process
//	root		- Path to the contextual root node for the resolution
//	current		- Path to the file system node from which to begin resolution
//	path		- Path string to be resolved
//	flags		- Path resolution flags (LINUX_O_XXXXX)

std::shared_ptr<FileSystem::Path> FileSystem::LookupPath(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root,
	std::shared_ptr<FileSystem::Path> current, const char_t* path, int flags)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(*path == 0) throw LinuxException{ LINUX_ENOENT };

	// Use the private version of this function that accepts the recursion depth
	return LookupPath(ns, root, current, path, FileSystem::LookupFlags(flags), 0);
}

//-----------------------------------------------------------------------------
// FileSystem::LookupPath (private)
//
// Executes a path lookup operation
//
// Arguments:
//
//	ns			- Namespace associated with the calling process
//	root		- Path to the contextual root node for the resolution
//	current		- Path to the file system node from which to begin resolution
//	path		- Path string to be resolved
//	flags		- Path resolution flags
//	depth		- Current lookup recursion depth (symbolic links)

std::shared_ptr<FileSystem::Path> FileSystem::LookupPath(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
	std::shared_ptr<FileSystem::Path> current, const char_t* path, FileSystem::LookupFlags flags, int depth)
{
	// Increment and verify the recursion depth of the current lookup
	if(++depth >= FileSystem::MaxSymbolicLinks) throw LinuxException{ LINUX_ELOOP };

	// Convert the path string into a posix_path instance and iterate over it
	auto lookuppath = posix_path(path);
	for(const auto& iterator : lookuppath) {

		// Get the parent of the current component, this can be null which indicates that the
		// component is a self-referential root path (".." leads to itself)
		auto parent = (current->m_parent) ? current->m_parent : current;
		auto node = current->m_alias->Node;

		// ROOT [/]: switch current to the root alias and mount
		if(strcmp(iterator, "/") == 0) { current = root; continue; }

		// SELF [.]: skip to the next path component
		if(strcmp(iterator, ".") == 0) continue;

		// PARENT [..] move to the current component's parent path instance
		if(strcmp(iterator, "..") == 0) { current = parent; continue; }

		// If the current path component is a symbolic link, follow it (must result in a directory)
		if(node->Type == FileSystem::NodeType::SymbolicLink) {

			auto symlink = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(node);
			if(!symlink) throw LinuxException{ LINUX_ENOENT };

			current = LookupPath(ns, root, parent, symlink->Target.c_str(), FileSystem::LookupFlags::Directory, depth);
		}

		// If the current path component is a directory, look up the child path component
		else if(node->Type == FileSystem::NodeType::Directory) {

			auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(node);
			if(!directory) throw LinuxException{ LINUX_ENOTDIR };

			// Retrieve the alias for the child object and check if it is a mount point
			auto newalias = directory->Lookup(current->m_mount, iterator);
			auto newmount = ns->Mounts->Find(newalias);

			// Create the updated Path based on the new alias and optionally the new mount point
			current = std::make_shared<FileSystem::Path>(current, newalias, (newmount) ? newmount : current->m_mount);
		}

		else throw LinuxException{ LINUX_ENOTDIR };
	}

	// If the final path component is a symbolic link, the O_NOFOLLOW flag must be checked
	if(current->m_alias->Node->Type == FileSystem::NodeType::SymbolicLink) {

		// O_NOFOLLOW - return the symbolic link as the final component
		if(flags & FileSystem::LookupFlags::NoFollow) return current;

		// Not O_FOLLOW - read the symbolic link target and follow it (relative to parent)
		auto symlink = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(current->m_alias->Node);
		if(!symlink) throw LinuxException{ LINUX_ENOENT };

		current = LookupPath(ns, root, current->m_parent, symlink->Target.c_str(), FileSystem::LookupFlags::None, depth);
	}

	// O_DIRECTORY indicates that the resolved path must indicate a directory object
	if((flags & FileSystem::LookupFlags::Directory) && (current->m_alias->Node->Type != FileSystem::NodeType::Directory))
		throw LinuxException{ LINUX_ENOTDIR };
	
	return current;
}

//-----------------------------------------------------------------------------
// FileSystem::OpenExecutable (static)
//
// Opens an executable file system object and returns a Handle instance
//
// Arguments:
//
//	ns			- Namespace in which to perform name resolution
//	root		- Path to the contextual root node for the resolution
//	current		- Path to the file system node from which to begin resolution
//	path		- Path to the object to be opened

std::shared_ptr<FileSystem::Handle> FileSystem::OpenExecutable(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root,
	std::shared_ptr<FileSystem::Path> current, const char_t* path)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(*path == 0) throw LinuxException{ LINUX_ENOENT };

	// Attempt to resolve the file system object, it must be a regular file
	auto exepath = LookupPath(ns, root, current, path, LookupFlags::None, 0);
	
	// Create and return an executable handle for the file system object
	auto file = std::dynamic_pointer_cast<FileSystem::File>(exepath->m_alias->Node);
	if(!file) throw LinuxException{ LINUX_ENOEXEC };

	return file->OpenExec(exepath->m_mount);
}

//-----------------------------------------------------------------------------
// FileSystem::OpenFile (static)
//
// Opens a file system object and returns a Handle instance
//
// Arguments:
//
//	ns			- Namespace in which to perform name resolution
//	root		- Path to the contextual root node for the resolution
//	current		- Path to the file system node from which to begin resolution
//	path		- Path to the object to be opened or created
//	flags		- Handle access mode and flags (LINUX_O_XXXXX)
//	mode		- Permissions to assign if a new object is created

std::shared_ptr<FileSystem::Handle> FileSystem::OpenFile(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
	std::shared_ptr<FileSystem::Path> current, const char_t* path, int flags, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(*path == 0) throw LinuxException{ LINUX_ENOENT };

	// Break the requested path up into branch and leaf components
	posix_path filepath(path);
	auto branch = filepath.branch();
	auto leaf = filepath.leaf();

	// Change to the branch path, which must resolve to a directory instance
	current = LookupPath(ns, root, current, branch, FileSystem::LookupFlags::Directory, 0);

	// Cast the branch directory into a FileSystem::Directory node instance
	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(current->m_alias->Node);
	if(!directory) throw LinuxException{ LINUX_ENOTDIR };

	// O_CREAT - Handle special rules regarding optional creation of a new regular file
	if(flags & LINUX_O_CREAT) {

		// If there is no leaf component, the operation is referring to a directory
		if(!leaf) throw LinuxException{ LINUX_EISDIR };

		// todo
		(mode);
		// check if the object exists -- add method to FileSystem::Directory
		// if exists and O_EXCL, throw
		// if exists and not O_EXCL, fall through
		// create a new regular file object and return the handle
	}

	// O_TMPFILE (| O_EXCL)
	// todo -- see open(2) for documentation, will need support in FileSystem::Directory

	// Lookup the final path component
	current = LookupPath(ns, root, current, leaf, FileSystem::LookupFlags(flags), 0);

	// O_PATH is handled by a special PathHandle object, otherwise request the handle from the located node instance
	if(flags & LINUX_O_PATH) return std::make_shared<PathHandle>(current->m_alias->Node, FileSystem::HandleAccess(flags));
	else return current->m_alias->Node->Open(current->m_mount, FileSystem::HandleAccess(flags), FileSystem::HandleFlags(flags));
}

//-----------------------------------------------------------------------------
// FileSystem::ReadSymbolicLink (static)
//
// Reads the target string from a file system symbolic link
//
// Arguments:
//
//	ns			- Namespace in which to perform name resolution
//	root		- Path to the contextual root node for the resolution
//	current		- Path to the file system node from which to begin resolution
//	path		- Path to the symbolic link object
//	buffer		- Target string output buffer
//	length		- Length of the target string output buffer

size_t FileSystem::ReadSymbolicLink(std::shared_ptr<Namespace> ns, std::shared_ptr<FileSystem::Path> root, 
		std::shared_ptr<FileSystem::Path> current, const char_t* path, char_t* buffer, size_t length)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(*path == 0) throw LinuxException{ LINUX_ENOENT };

	// Ensure that the buffer pointer is not null and is at least one byte in length
	if(buffer == nullptr) throw LinuxException{ LINUX_EFAULT };
	if(length == 0) throw LinuxException{ LINUX_EINVAL };

	// Attempt to resolve the file system object, do not follow a trailing symbolic link
	current = LookupPath(ns, root, current, path, LookupFlags::NoFollow, 0);

	// The provided path must have led to a symbolic link file system object
	auto symlink = std::dynamic_pointer_cast<FileSystem::SymbolicLink>(current->m_alias->Node);
	if(!symlink) throw LinuxException{ LINUX_EINVAL };

	// Read the target information from the symbolic link
	return symlink->GetTarget(buffer, length);
}

//
// FILESYSTEM::PATH
//

//-----------------------------------------------------------------------------
// FileSystem::Path Constructor (private)
//
// Arguments:
//
//	alias		- Alias object reference
//	mount		- Mount object reference

FileSystem::Path::Path(std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount) : 
	m_alias{ std::move(alias) }, m_mount{ std::move(mount) }
{
}

//-----------------------------------------------------------------------------
// FileSystem::Path Constructor (private)
//
// Arguments:
//
//	parent		- Parent path instance
//	alias		- Alias object reference
//	mount		- Mount object reference

FileSystem::Path::Path(std::shared_ptr<FileSystem::Path> parent, std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount) : 
	m_parent{ std::move(parent) }, m_alias{ std::move(alias) }, m_mount{ std::move(mount) }
{
}

////-----------------------------------------------------------------------------
//// FileSystem::Path Copy Constructor
//
//FileSystem::Path::Path(const FileSystem::Path& rhs) : 
//	m_parent{ rhs.m_parent }, m_alias{ rhs.m_alias }, m_mount{ rhs.m_mount }
//{
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::Path Move Constructor
//
//FileSystem::Path::Path(FileSystem::Path&& rhs) : 
//	m_parent{ std::move(rhs.m_parent) }, m_alias{ std::move(rhs.m_alias) }, m_mount{ std::move(rhs.m_mount) }
//{
//}

std::shared_ptr<FileSystem::Path> FileSystem::Path::Create(std::shared_ptr<FileSystem::Alias> alias, std::shared_ptr<FileSystem::Mount> mount)
{
	return std::make_shared<Path>(std::move(alias), std::move(mount));
}

//
// FILESYSTEM::PATHHANDLE
//

//-----------------------------------------------------------------------------
// FileSystem::PathHandle Constructor
//
// Arguments:
//
//	node		- Node instance to attach to the handle
//	access		- Handle access mode

FileSystem::PathHandle::PathHandle(std::shared_ptr<Node> node, FileSystem::HandleAccess access)
	: m_node{ std::move(node) }, m_access{ access }
{
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::getAccess
//
// Gets the handle access mode

FileSystem::HandleAccess FileSystem::PathHandle::getAccess(void) const
{
	return m_access;
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Duplicate
//
// Creates a duplicate Handle instance
//
// Arguments:
//
//	NONE

std::shared_ptr<FileSystem::Handle> FileSystem::PathHandle::Duplicate(void) const
{
	// Construct a new PathHandle linked to the same node and with the same access
	return std::make_shared<PathHandle>(m_node, m_access);
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::getFlags
//
// Gets the flags specified on the handle

FileSystem::HandleFlags FileSystem::PathHandle::getFlags(void) const
{
	return HandleFlags::None;
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Destination buffer
//	count		- Maximum number of bytes to read

uapi::size_t FileSystem::PathHandle::Read(void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException{ LINUX_EBADF };
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::ReadAt
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	offset		- Absolute file position to begin reading from
//	buffer		- Destination buffer
//	count		- Maximum number of bytes to read

uapi::size_t FileSystem::PathHandle::ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException{ LINUX_EBADF };
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Seek
//
// Changes the file position
//
// Arguments:
//
//	offset		- Offset (relative to whence) to position the file pointer
//	whence		- Flag indicating the file position from which offset applies

uapi::loff_t FileSystem::PathHandle::Seek(uapi::loff_t offset, int whence)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(whence);

	throw LinuxException{ LINUX_EBADF };
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void FileSystem::PathHandle::Sync(void) const
{
	throw LinuxException{ LINUX_EBADF };
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void FileSystem::PathHandle::SyncData(void) const
{
	throw LinuxException{ LINUX_EBADF };
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer		- Source buffer
//	count		- Maximum number of bytes to write

uapi::size_t FileSystem::PathHandle::Write(const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);
	
	throw LinuxException{ LINUX_EBADF };
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::WriteAt
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	offset		- Absolute file position to begin writing from
//	buffer		- Source buffer
//	count		- Maximum number of bytes to write

uapi::size_t FileSystem::PathHandle::WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException{ LINUX_EBADF };
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
