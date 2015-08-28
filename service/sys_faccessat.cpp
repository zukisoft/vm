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

#include "Context.h"
#include "FileSystem.h"
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_faccessat
//
// Checks permissions to a file system object
//
// Arguments:
//
//	context		- System call context object
//	dirfd		- Previously opened directory object file descriptor
//	pathname	- Relative path for the file system object to check
//	mode		- Accessibility check mask (F_OK, R_OK, etc)
//	flags		- Behavioral flags

uapi::long_t sys_faccessat(const Context* context, int dirfd, const uapi::char_t* pathname, uapi::mode_t mode, int flags)
{
	// Determine if an absolute or relative pathname has been provided
	bool absolute = ((pathname) && (pathname[0] == '/'));

	// Determine the base alias from which to resolve the path
	FileSystem::AliasPtr base = absolute ? context->Process->RootDirectory : 
		((dirfd == LINUX_AT_FDCWD) ? context->Process->WorkingDirectory : context->Process->Handle[dirfd]->Alias);

	// NOTE TODO NOTE TODO
	// This function operates against the REAL uid and gid of the process, not the effective UID and GID;
	// those are the values typically examined by a file system operation like open(2).
	// Same thing applies to capabilities, it's the real ones not the effective ones
	// there will need to be some way to strip down a call automatically from effective to real and then
	// invoke FileSystem::CheckPermissions() or whatever the new call ends up being

	// Use the _VmOld interface to check permissions to the specified file system object
	FileSystem::CheckPermissions(context->Process->RootDirectory, base, pathname, flags, mode);

	return 0;
}

// sys32_faccessat
//
sys32_long_t sys32_faccessat(sys32_context_t context, sys32_int_t dirfd, const sys32_char_t* pathname, sys32_mode_t mode, sys32_int_t flags)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_faccessat, context, dirfd, pathname, mode, flags));
}

#ifdef _M_X64
// sys64_faccessat
//
sys64_long_t sys64_faccessat(sys64_context_t context, sys64_int_t dirfd, const sys64_char_t* pathname, sys64_mode_t mode, sys64_int_t flags)
{
	return SystemCall::Invoke(sys_faccessat, context, dirfd, pathname, mode, flags);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
