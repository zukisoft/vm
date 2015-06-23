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
//#include "Namespace.h"
#include "Random.h"

#pragma warning(push, 4)

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
//
////-----------------------------------------------------------------------------
//// FileSystem::OpenExecutable (static)
////
//// Opens an executable file system object and returns a Handle instance
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be opened/created
//
//std::shared_ptr<FileSystem::Handle> FileSystem::OpenExecutable(const std::shared_ptr<Alias>& root, 
//	const std::shared_ptr<Alias>& base, const char_t* path)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// Attempt to resolve the file system object's name
//	auto alias = ResolvePath(root, base, path, 0);
//	
//	// The only valid node type is a regular file object
//	auto node = std::dynamic_pointer_cast<File>(alias->Node);
//	if(node == nullptr) throw LinuxException(LINUX_ENOEXEC);
//
//	// Create and return an executable handle for the file system object
//	return node->OpenExec(alias);
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::OpenFile (static)
////
//// Opens a file system object and returns a Handle instance
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be opened/created
////	flags		- Flags indicating how the object should be opened
////	mode		- Mode bitmask to use if a new object is created
//
//std::shared_ptr<FileSystem::Handle> FileSystem::OpenFile(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
//	const char_t* path, int flags, uapi::mode_t mode)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// O_PATH filter -> Only O_CLOEXEC, O_DIRECTORY and O_NOFOLLOW are evaluated
//	if(flags & LINUX_O_PATH) flags &= (LINUX_O_PATH | LINUX_O_CLOEXEC | LINUX_O_DIRECTORY | LINUX_O_NOFOLLOW);
//
//	// O_CREAT | O_EXCL indicates that a regular file object must be created, call CreateFile() instead.
//	if((flags & (LINUX_O_CREAT | LINUX_O_EXCL)) == (LINUX_O_CREAT | LINUX_O_EXCL)) return CreateFile(root, base, path, flags, mode);
//
//	// O_CREAT indicates that if the object does not exist, a new regular file will be created
//	else if((flags & LINUX_O_CREAT) == LINUX_O_CREAT) {
//
//		PathSplitter splitter(path);				// Path splitter
//
//		// Resolve the branch path to an Alias instance, must resolve to a Directory
//		auto branch = ResolvePath(root, base, splitter.Branch, flags);
//		if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
//
//		// Ask the branch node to resolve the leaf, if that succeeds, just open it
//		try { 
//			
//			auto leaf = ResolvePath(root, branch, splitter.Leaf, flags);
//			return leaf->Node->Open(leaf, flags); 
//		}
//		
//		catch(...) { /* DON'T CARE - KEEP GOING */ }
//
//		// The leaf didn't exist (or some other bad thing happened), cast out the branch
//		// as a Directory node and attempt to create the file instead
//		auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
//		if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);
//
//		return directory->CreateFile(branch, splitter.Leaf, flags, mode);
//	}
//
//	// Standard open, will throw exception if the object does not exist
//	auto alias = ResolvePath(root, base, path, flags);
//	return alias->Node->Open(alias, flags);
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::ReadSymbolicLink (static)
////
//// Reads the target string from a file system symbolic link
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path to the object to be opened/created
////	buffer		- Target string output buffer
////	length		- Length of the target string output buffer
//
//size_t FileSystem::ReadSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const char_t* path, 
//	char_t* buffer, size_t length)
//{
//	// per path_resolution(7), empty paths are not allowed
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// Ensure that the buffer pointer is not null and is at least one byte in length
//	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//	if(length == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));
//
//	// Find the desired symbolic link in the file system
//	auto alias = ResolvePath(root, base, path, LINUX_O_NOFOLLOW);
//	
//	// The only valid node type is a symbolic link object; EINVAL if it's not
//	auto symlink = std::dynamic_pointer_cast<SymbolicLink>(alias->Node);
//	if(symlink == nullptr) throw LinuxException(LINUX_EINVAL);
//
//	return symlink->ReadTarget(buffer, length);
//}
//
////-----------------------------------------------------------------------------
//// FileSystem::ResolvePath (static)
////
//// Resolves a file system path to an alias instance
////
//// Arguments:
////
////	root		- Root alias to use for path resolution
////	base		- Base alias from which to start path resolution
////	path		- Path string to be resolved
////	flags		- Path resolution flags
//
//std::shared_ptr<FileSystem::Alias> FileSystem::ResolvePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
//	const char_t* path, int flags)
//{
//	int	symlinks = 0;							// Number of symbolic links encountered
//
//	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
//
//	// If there is no path to consume, resolve to the base alias rather than
//	// raising ENOENT.  This is a valid operation when a parent directory needs
//	// to be resolved and it happens to be the base alias
//	if(*path == 0) return base;
//
//	// Determine if the path was absolute or relative and remove leading slashes
//	bool absolute = (*path == '/');
//	while(*path == '/') path++;
//
//	// Start at either the root or the base depending on the path type, and ask
//	// that node to resolve the now relative path string to an Alias instance
//	auto current = (absolute) ? root : base;
//	return current->Node->Resolve(root, current, path, flags, &symlinks);
//}
	
//
// FILESYSTEM::EXECUTEHANDLE
//

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle Constructor (private)
//
// Arguments:
//
//	handle		- Reference to the custom file system handle to wrap

FileSystem::ExecuteHandle::ExecuteHandle(const std::shared_ptr<FileSystem::Handle>& handle) : m_handle(handle)
{
	_ASSERTE(handle);
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::Create (static)
//
// Creates a new ExecuteHandle instance, wrapping an existing file system handle
//
// Arguments:
//
//	handle		- Reference to the custom file system handle to wrap

std::shared_ptr<FileSystem::Handle> FileSystem::ExecuteHandle::Create(const std::shared_ptr<FileSystem::Handle>& handle)
{
	return std::make_shared<ExecuteHandle>(handle);
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::Duplicate
//
// Creates a duplicate Handle instance
//
// Arguments:
//
//	flags		- Flags applicable to the new handle

std::shared_ptr<FileSystem::Handle> FileSystem::ExecuteHandle::Duplicate(int flags)
{
	return m_handle->Duplicate(flags);
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::Read
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	buffer		- Destination buffer
//	count		- Maximum number of bytes to read

uapi::size_t FileSystem::ExecuteHandle::Read(void* buffer, uapi::size_t count)
{
	return m_handle->Read(buffer, count);
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::ReadAt
//
// Synchronously reads data from the underlying node into a buffer
//
// Arguments:
//
//	offset		- Absolute file position to begin reading from
//	buffer		- Destination buffer
//	count		- Maximum number of bytes to read

uapi::size_t FileSystem::ExecuteHandle::ReadAt(uapi::loff_t offset, void* buffer, uapi::size_t count)
{
	return m_handle->ReadAt(offset, buffer, count);
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::Seek
//
// Changes the file position
//
// Arguments:
//
//	offset		- Offset (relative to whence) to position the file pointer
//	whence		- Flag indicating the file position from which offset applies

uapi::loff_t FileSystem::ExecuteHandle::Seek(uapi::loff_t offset, int whence)
{
	return m_handle->Seek(offset, whence);
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void FileSystem::ExecuteHandle::Sync(void)
{
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void FileSystem::ExecuteHandle::SyncData(void)
{
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::Write
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	buffer		- Source buffer
//	count		- Maximum number of bytes to write

uapi::size_t FileSystem::ExecuteHandle::Write(const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);
	
	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// FileSystem::ExecuteHandle::WriteAt
//
// Synchronously writes data from a buffer to the underlying node
//
// Arguments:
//
//	offset		- Absolute file position to begin writing from
//	buffer		- Source buffer
//	count		- Maximum number of bytes to write

uapi::size_t FileSystem::ExecuteHandle::WriteAt(uapi::loff_t offset, const void* buffer, uapi::size_t count)
{
	UNREFERENCED_PARAMETER(offset);
	UNREFERENCED_PARAMETER(buffer);
	UNREFERENCED_PARAMETER(count);

	throw LinuxException(LINUX_EACCES, Exception(E_NOTIMPL));
}

//
// FILESYSTEM::PATH
//

//-----------------------------------------------------------------------------
// FileSystem::Path Constructor
//
// Arguments:
//
//	mount		- Mount object reference
//	alias		- Alias object reference

FileSystem::Path::Path(const std::shared_ptr<FileSystem::Mount>& mount, const std::shared_ptr<FileSystem::Alias>& alias) :
	m_mount(mount), m_alias(alias)
{
	_ASSERTE(mount);
	_ASSERTE(alias);
}

//-----------------------------------------------------------------------------
// FileSystem::Path::getAlias
//
// Gets a reference to the contained Alias instance

std::shared_ptr<FileSystem::Alias> FileSystem::Path::getAlias(void) const
{
	return m_alias;
}

//-----------------------------------------------------------------------------
// FileSystem::Path::Create (static)
//
// Creates a new Path instance
//
// Arguments:
//
//	mount		- Mount object reference
//	alias		- Alias object reference

std::unique_ptr<FileSystem::Path> FileSystem::Path::Create(const std::shared_ptr<FileSystem::Mount>& mount, const std::shared_ptr<FileSystem::Alias>& alias)
{
	return std::make_unique<Path>(mount, alias);
}

//-----------------------------------------------------------------------------
// FileSystem::Path::Duplicate
//
// Duplicates this Path instance
//
// Arguments:
//
//	NONE

std::unique_ptr<FileSystem::Path> FileSystem::Path::Duplicate(void) const
{
	return std::make_unique<Path>(m_mount, m_alias);
}

//-----------------------------------------------------------------------------
// FileSystem::Path::getMount
//
// Gets a reference to the contained Mount instance

std::shared_ptr<FileSystem::Mount> FileSystem::Path::getMount(void) const
{
	return m_mount;
}

//
// FILESYSTEM::PATHHANDLE
//

//-----------------------------------------------------------------------------
// FileSystem::PathHandle Constructor (private)
//
// Arguments:
//
//	handle		- Reference to the custom file system handle to wrap

FileSystem::PathHandle::PathHandle(const std::shared_ptr<FileSystem::Handle>& handle) : m_handle(handle)
{
	_ASSERTE(handle);
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Create (static)
//
// Creates a new PathHandle instance, wrapping an existing file system handle
//
// Arguments:
//
//	handle		- Reference to the custom file system handle to wrap

std::shared_ptr<FileSystem::Handle> FileSystem::PathHandle::Create(const std::shared_ptr<FileSystem::Handle>& handle)
{
	return std::make_shared<PathHandle>(handle);
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Duplicate
//
// Creates a duplicate Handle instance
//
// Arguments:
//
//	flags		- Flags applicable to the new handle

std::shared_ptr<FileSystem::Handle> FileSystem::PathHandle::Duplicate(int flags)
{
	return m_handle->Duplicate(flags);
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

	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
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

	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
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

	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::Sync
//
// Synchronizes all metadata and data associated with the file to storage
//
// Arguments:
//
//	NONE

void FileSystem::PathHandle::Sync(void)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------
// FileSystem::PathHandle::SyncData
//
// Synchronizes all data associated with the file to storage, not metadata
//
// Arguments:
//
//	NONE

void FileSystem::PathHandle::SyncData(void)
{
	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
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

	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
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

	throw LinuxException(LINUX_EBADF, Exception(E_NOTIMPL));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
