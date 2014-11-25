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
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_fstatat64
//
// Get information and statistics about a file system object
//
// Arguments:
//
//	context		- SystemCall context object
//	fd			- Previously opened directory object file descriptor
//	pathname	- Relative path for the file system object to access
//	stats		- Output structure
//	flags		- Path resolution flags (AT_EMPTY_PATH, etc)

__int3264 sys_fstatat64(const SystemCall::Context* context, int fd, const uapi::char_t* pathname, linux_stat3264* buf, int flags)
{
	uapi::stat				stats;				// File system object statistics

	_ASSERTE(context);
	if(buf == nullptr) return -LINUX_EFAULT;

	// Verify the flags are valid for this operation
	if((flags & ~(LINUX_AT_SYMLINK_NOFOLLOW | LINUX_AT_NO_AUTOMOUNT | LINUX_AT_EMPTY_PATH)) != 0) return -LINUX_EINVAL;

	try {

		SystemCall::Impersonation impersonation;

		// Determine if an absolute or relative pathname has been provided
		bool absolute = ((pathname) && (pathname[0] == '/'));

		// Determine the base alias from which to resolve the path
		FileSystem::AliasPtr base = absolute ? context->Process->RootDirectory : 
			((fd == LINUX_AT_FDCWD) ? context->Process->WorkingDirectory : context->Process->GetHandle(fd)->Alias);

		// Base for path resolution must be a directory if LINUX_AT_EMPTY_PATH is not specified
		if(((flags & LINUX_AT_EMPTY_PATH) != LINUX_AT_EMPTY_PATH) && (base->Node->Type != FileSystem::NodeType::Directory))
			throw LinuxException(LINUX_ENOTDIR);

		// Get the generic information and statistics for the node
		//context->VirtualMachine->ResolvePath(context->Process->RootDirectory, base, pathname, 
		//	(flags & LINUX_AT_SYMLINK_NOFOLLOW) ? LINUX_O_NOFOLLOW : 0)->Node->Stat(&stats);

		//
		// CONVERT DATA STRUCTURE
		// WATCH FOR E_OVERFLOW
		//
		//memset(buf, 0, sizeof(linux_stat3264));
		(stats);
		throw LinuxException(LINUX_ENOSYS);
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return 0;
}

// sys32_fstatat64
//
sys32_long_t sys32_fstatat64(sys32_context_t context, sys32_int_t fd, const sys32_char_t* pathname, linux_stat3264* buf, sys32_int_t flags)
{
	return static_cast<sys32_long_t>(sys_fstatat64(reinterpret_cast<SystemCall::Context*>(context), fd, pathname, buf, flags));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
