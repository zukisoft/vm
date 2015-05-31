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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// FileSystem::BlockDeviceBase::getType (private)
//
// Gets the type of node defined by this object

FileSystem::NodeType FileSystem::BlockDeviceBase::getType(void)
{
	return FileSystem::NodeType::BlockDevice;
}

//-----------------------------------------------------------------------------
// FileSystem::CharacterDeviceBase::getType (private)
//
// Gets the type of node defined by this object

FileSystem::NodeType FileSystem::CharacterDeviceBase::getType(void)
{
	return FileSystem::NodeType::CharacterDevice;
}

//-----------------------------------------------------------------------------
// FileSystem::DirectoryBase::getType (private)
//
// Gets the type of node defined by this object

FileSystem::NodeType FileSystem::DirectoryBase::getType(void)
{
	return FileSystem::NodeType::Directory;
}

//-----------------------------------------------------------------------------
// FileSystem::FileBase::getType (private)
//
// Gets the type of node defined by this object

FileSystem::NodeType FileSystem::FileBase::getType(void)
{
	return FileSystem::NodeType::File;
}

//-----------------------------------------------------------------------------
// FileSystem::PipeBase::getType (private)
//
// Gets the type of node defined by this object

FileSystem::NodeType FileSystem::PipeBase::getType(void)
{
	return FileSystem::NodeType::Pipe;
}

//-----------------------------------------------------------------------------
// FileSystem::SocketBase::getType (private)
//
// Gets the type of node defined by this object

FileSystem::NodeType FileSystem::SocketBase::getType(void)
{
	return FileSystem::NodeType::Socket;
}

//-----------------------------------------------------------------------------
// FileSystem::SymbolicLinkBase::getType (private)
//
// Gets the type of node defined by this object

FileSystem::NodeType FileSystem::SymbolicLinkBase::getType(void)
{
	return FileSystem::NodeType::SymbolicLink;
}

//-----------------------------------------------------------------------------
// FileSystem::CheckPermissions (static)
//
// Demands read/write/execute permissions for a file system object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be checked
//	flags		- Operation flags (AT_EACCESS, AT_SYMLINK_NO_FOLLOW)
//	mode		- Special MAY_READ, MAY_WRITE, MAY_EXECUTE flags

void FileSystem::CheckPermissions(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, 
	int flags, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Default behavior is to dereference symbolic links, sys_faccessat() can specify otherwise
	int resolveflags = ((flags & LINUX_AT_SYMLINK_NOFOLLOW) == LINUX_AT_SYMLINK_NOFOLLOW) ? LINUX_O_NOFOLLOW : 0;

	// TODO: support AT_EACCESS flag

	// Demand the requested permission(s) from the target file system node
	ResolvePath(root, base, path, resolveflags)->Node->DemandPermission(mode);
}

//-----------------------------------------------------------------------------
// FileSystem::CreateCharacterDevice (static)
//
// Creates a file system character device object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	mode		- Mode bitmask to use when creating the object
//	device		- Target device identifier

void FileSystem::CreateCharacterDevice(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, 
	uapi::mode_t mode, uapi::dev_t device)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, 0);
	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	// Request a new character device node be created as a child of the resolved directory
	directory->CreateCharacterDevice(branch, splitter.Leaf, mode, device);
}

//-----------------------------------------------------------------------------
// FileSystem::CreateDirectory (static)
//
// Creates a file system directory object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	mode		- Mode bitmask to use if a new object is created

void FileSystem::CreateDirectory(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
	const uapi::char_t* path, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, 0);
	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
	
	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	directory->CreateDirectory(branch, splitter.Leaf, mode);
}

//-----------------------------------------------------------------------------
// FileSystem::CreateFile (static)
//
// Creates a regular file object and returns a Handle instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	flags		- Flags indicating how the object should be opened
//	mode		- Mode bitmask to use if a new object is created

std::shared_ptr<FileSystem::Handle> FileSystem::CreateFile(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
	const uapi::char_t* path, int flags, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// O_PATH and O_DIRECTORY cannot be used when creating a regular file object
	if((flags & LINUX_O_PATH) || (flags & LINUX_O_DIRECTORY)) throw LinuxException(LINUX_EINVAL);

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, flags);
	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	// Request a new regular file be created as a child of the resolved directory
	return directory->CreateFile(branch, splitter.Leaf, flags, mode);
}

//-----------------------------------------------------------------------------
// FileSystem::CreateSymbolicLink
//
// Creates a file system symbolic link object
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	target		- Symbolic link target

void FileSystem::CreateSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, 
	const uapi::char_t* target)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Split the path into branch and leaf components
	PathSplitter splitter(path);

	// Resolve the branch path to an Alias instance, must resolve to a Directory
	auto branch = ResolvePath(root, base, splitter.Branch, 0);
	if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);
	
	auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	directory->CreateSymbolicLink(branch, splitter.Leaf, target);
}

//-----------------------------------------------------------------------------
// FileSystem::GetAbsolutePath (static)
//
// Retrieves the absolute path for a provided relative path
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Relative path to be converted into an absolute path

void FileSystem::GetAbsolutePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& alias, uapi::char_t* path, size_t pathlen)
{
	std::vector<std::string> pathvec;

	if(path == nullptr) throw LinuxException(LINUX_EFAULT);
	if(pathlen == 0) throw LinuxException(LINUX_ERANGE);

	// TODO: not sure what to do if the current directory has been deleted (unlinked)

	// Start at the specified file system alias and continue working backwards until a root node is found
	std::shared_ptr<Alias> current = alias;
	while(current->Parent != current) {

		// If the current node is a symbolic link, follow it to the target and loop again
		if(current->Node->Type == NodeType::SymbolicLink) {

			auto symlink = std::dynamic_pointer_cast<SymbolicLink>(current->Node);

			// if this throws, should it be "(unreachable)" -- see kernel code
			int loop = 0;	// For ELOOP detection
			current = symlink->Resolve(root, current, nullptr, 0, &loop);
			continue;
		}

		// Should never happen, but check for it regardless
		if(current->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

		// Push the next Alias name into the path building collection and move up to parent
		pathvec.push_back(current->Name);
		current = current->Parent;
	}

	pathvec.push_back("");

	std::string tododeleteme;
	for (auto iterator = pathvec.begin(); iterator != pathvec.end(); iterator++) {
			
		tododeleteme += "/";
		tododeleteme += *iterator;
	}

	if(tododeleteme.length() + 1 > pathlen) throw LinuxException(LINUX_ERANGE);
	strncpy_s(path, pathlen, tododeleteme.c_str(), _TRUNCATE);
}

//-----------------------------------------------------------------------------
// FileSystem::OpenExecutable (static)
//
// Opens an executable file system object and returns a Handle instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created

std::shared_ptr<FileSystem::Handle> FileSystem::OpenExecutable(const std::shared_ptr<Alias>& root, 
	const std::shared_ptr<Alias>& base, const uapi::char_t* path)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Attempt to resolve the file system object's name
	auto alias = ResolvePath(root, base, path, 0);
	
	// The only valid node type is a regular file object
	auto node = std::dynamic_pointer_cast<File>(alias->Node);
	if(node == nullptr) throw LinuxException(LINUX_ENOEXEC);

	// Create and return an executable handle for the file system object
	return node->OpenExec(alias);
}

//-----------------------------------------------------------------------------
// FileSystem::OpenFile (static)
//
// Opens a file system object and returns a Handle instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	flags		- Flags indicating how the object should be opened
//	mode		- Mode bitmask to use if a new object is created

std::shared_ptr<FileSystem::Handle> FileSystem::OpenFile(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
	const uapi::char_t* path, int flags, uapi::mode_t mode)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// O_PATH filter -> Only O_CLOEXEC, O_DIRECTORY and O_NOFOLLOW are evaluated
	if(flags & LINUX_O_PATH) flags &= (LINUX_O_PATH | LINUX_O_CLOEXEC | LINUX_O_DIRECTORY | LINUX_O_NOFOLLOW);

	// O_CREAT | O_EXCL indicates that a regular file object must be created, call CreateFile() instead.
	if((flags & (LINUX_O_CREAT | LINUX_O_EXCL)) == (LINUX_O_CREAT | LINUX_O_EXCL)) return CreateFile(root, base, path, flags, mode);

	// O_CREAT indicates that if the object does not exist, a new regular file will be created
	else if((flags & LINUX_O_CREAT) == LINUX_O_CREAT) {

		PathSplitter splitter(path);				// Path splitter

		// Resolve the branch path to an Alias instance, must resolve to a Directory
		auto branch = ResolvePath(root, base, splitter.Branch, flags);
		if(branch->Node->Type != NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

		// Ask the branch node to resolve the leaf, if that succeeds, just open it
		try { 
			
			auto leaf = ResolvePath(root, branch, splitter.Leaf, flags);
			return leaf->Node->Open(leaf, flags); 
		}
		
		catch(...) { /* DON'T CARE - KEEP GOING */ }

		// The leaf didn't exist (or some other bad thing happened), cast out the branch
		// as a Directory node and attempt to create the file instead
		auto directory = std::dynamic_pointer_cast<Directory>(branch->Node);
		if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

		return directory->CreateFile(branch, splitter.Leaf, flags, mode);
	}

	// Standard open, will throw exception if the object does not exist
	auto alias = ResolvePath(root, base, path, flags);
	return alias->Node->Open(alias, flags);
}

//-----------------------------------------------------------------------------
// FileSystem::ReadSymbolicLink (static)
//
// Reads the target string from a file system symbolic link
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created
//	buffer		- Target string output buffer
//	length		- Length of the target string output buffer

size_t FileSystem::ReadSymbolicLink(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, const uapi::char_t* path, 
	uapi::char_t* buffer, size_t length)
{
	// per path_resolution(7), empty paths are not allowed
	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(*path == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Ensure that the buffer pointer is not null and is at least one byte in length
	if(buffer == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));
	if(length == 0) throw LinuxException(LINUX_ENOENT, Exception(E_INVALIDARG));

	// Find the desired symbolic link in the file system
	auto alias = ResolvePath(root, base, path, LINUX_O_NOFOLLOW);
	
	// The only valid node type is a symbolic link object; EINVAL if it's not
	auto symlink = std::dynamic_pointer_cast<SymbolicLink>(alias->Node);
	if(symlink == nullptr) throw LinuxException(LINUX_EINVAL);

	return symlink->ReadTarget(buffer, length);
}

//-----------------------------------------------------------------------------
// FileSystem::ResolvePath (static)
//
// Resolves a file system path to an alias instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path string to be resolved
//	flags		- Path resolution flags

std::shared_ptr<FileSystem::Alias> FileSystem::ResolvePath(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
	const uapi::char_t* path, int flags)
{
	int	symlinks = 0;							// Number of symbolic links encountered

	if(path == nullptr) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// If there is no path to consume, resolve to the base alias rather than
	// raising ENOENT.  This is a valid operation when a parent directory needs
	// to be resolved and it happens to be the base alias
	if(*path == 0) return base;

	// Determine if the path was absolute or relative and remove leading slashes
	bool absolute = (*path == '/');
	while(*path == '/') path++;

	// Start at either the root or the base depending on the path type, and ask
	// that node to resolve the now relative path string to an Alias instance
	auto current = (absolute) ? root : base;
	return current->Node->Resolve(root, current, path, flags, &symlinks);
}
	
//-----------------------------------------------------------------------------

#pragma warning(pop)
