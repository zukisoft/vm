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

#pragma warning(push, 4)

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
	if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

	auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
	if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

	// Request a new regular file be created as a child of the resolved directory
	return directory->CreateFile(branch, splitter.Leaf, flags, mode);
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
		if(branch->Node->Type != FileSystem::NodeType::Directory) throw LinuxException(LINUX_ENOTDIR);

		// Ask the branch node to resolve the leaf, if that succeeds, just open it
		try { 
			
			auto leaf = ResolvePath(root, branch, splitter.Leaf, flags);
			return leaf->Node->Open(leaf, flags); 
		}
		
		catch(...) { /* DON'T CARE - KEEP GOING */ }

		// The leaf didn't exist (or some other bad thing happened), cast out the branch
		// as a Directory node and attempt to create the file instead
		auto directory = std::dynamic_pointer_cast<FileSystem::Directory>(branch->Node);
		if(directory == nullptr) throw LinuxException(LINUX_ENOTDIR);

		return directory->CreateFile(branch, splitter.Leaf, flags, mode);
	}

	// Standard open, will throw exception if the object does not exist
	auto alias = ResolvePath(root, base, path, flags);
	return alias->Node->Open(alias, flags);
}

//-----------------------------------------------------------------------------
// FileSystem::ResolvePath (private, static)
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
