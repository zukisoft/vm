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
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_fstatat64
//
// Get information and statistics about a file system object
//
// Arguments:
//
//	context		- System call context object
//	fd			- Previously opened directory object file descriptor
//	pathname	- Relative path for the file system object to access
//	stats		- Output structure
//	flags		- Path resolution flags (AT_EMPTY_PATH, etc)

uapi::long_t sys_fstatat64(const Context* context, int fd, const uapi::char_t* pathname, linux_stat3264* buf, int flags)
{
	if(buf == nullptr) return -LINUX_EFAULT;

	// Verify the flags are valid for this operation
	if((flags & ~(LINUX_AT_SYMLINK_NOFOLLOW | LINUX_AT_NO_AUTOMOUNT | LINUX_AT_EMPTY_PATH)) != 0) return -LINUX_EINVAL;

	// Determine if an absolute or relative pathname has been provided
	bool absolute = ((pathname) && (pathname[0] == '/'));

	// Determine the base alias from which to resolve the path
	FileSystem::AliasPtr base = absolute ? context->Process->RootDirectory : 
		((fd == LINUX_AT_FDCWD) ? context->Process->WorkingDirectory : context->Process->GetHandle(fd)->Alias);

	// Base for path resolution must be a directory if LINUX_AT_EMPTY_PATH is not specified
	if(((flags & LINUX_AT_EMPTY_PATH) != LINUX_AT_EMPTY_PATH) && (base->Node->Type != FileSystem::NodeType::Directory))
		throw LinuxException(LINUX_ENOTDIR);

	// Attempt to resolve the target node
	auto node = context->VirtualMachine->ResolvePath(context->Process->RootDirectory, base, pathname, 
		(flags & LINUX_AT_SYMLINK_NOFOLLOW) ? LINUX_O_NOFOLLOW : 0)->Node;

	// Retrieve and convert the status information for the node
	uapi::stat status = node->Status;
	buf->st_dev			= status.st_dev;
	buf->st_ino			= status.st_ino;
	buf->st_nlink		= static_cast<uint32_t>(status.st_nlink);
	buf->st_mode		= status.st_mode;
	buf->st_uid			= status.st_uid;
	buf->st_gid			= status.st_gid;
	buf->st_rdev		= status.st_rdev;
	buf->st_size		= status.st_size;
	buf->st_blksize		= static_cast<uint32_t>(status.st_blksize);
	buf->st_blocks		= status.st_blocks;
	buf->st_atime		= static_cast<uint32_t>(status.st_atime);
	buf->st_atime_nsec	= static_cast<uint32_t>(status.st_atime_nsec);
	buf->st_mtime		= static_cast<uint32_t>(status.st_mtime);
	buf->st_mtime_nsec	= static_cast<uint32_t>(status.st_mtime_nsec);
	buf->st_ctime		= static_cast<uint32_t>(status.st_ctime);
	buf->st_ctime_nsec	= static_cast<uint32_t>(status.st_ctime_nsec);

	return 0;
}

// sys32_fstatat64
//
sys32_long_t sys32_fstatat64(sys32_context_t context, sys32_int_t fd, const sys32_char_t* pathname, linux_stat3264* buf, sys32_int_t flags)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_fstatat64, context, fd, pathname, buf, flags));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
