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
// FileSystem::OpenExecutable (static)
//
// Opens an executable file system object and returns a Handle instance
//
// Arguments:
//
//	root		- Root alias to use for path resolution
//	base		- Base alias from which to start path resolution
//	path		- Path to the object to be opened/created

std::shared_ptr<FileSystem::Handle> FileSystem::OpenExecutable(const std::shared_ptr<Alias>& root, const std::shared_ptr<Alias>& base, 
	const uapi::char_t* path)
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
