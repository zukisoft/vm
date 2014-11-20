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
#include "PathSplitter.h"
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_openat
//
// Opens, and possibly creates, a file with a path relative to an open directory
//
// Arguments:
//
//	context		- SystemCall context object
//	fd			- Previously opened directory object file descriptor
//	pathname	- Relative path for the file system object to open
//	flags		- Open operation flags
//	mode		- Mode flags to assign when creating a new file system object

__int3264 sys_openat(const SystemCall::Context* context, int fd, const uapi::char_t* pathname, int flags, uapi::mode_t mode)
{
	_ASSERTE(context);

	try {

		SystemCall::Impersonation impersonation;

		// Determine if an absolute or relative pathname has been provided
		bool absolute = ((pathname) && (pathname[0] == '/'));

		// Determine the base alias from which to resolve the path
		FileSystem::AliasPtr base = absolute ? context->Process->RootDirectory : 
			((fd == LINUX_AT_FDCWD) ? context->Process->WorkingDirectory : context->Process->GetHandle(fd)->Alias);

		// Attempt to open the file system object relative from the base alias
		return context->Process->AddHandle(context->VirtualMachine->OpenFile(context->Process->RootDirectory, base, pathname, flags, mode));
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_openat
//
sys32_long_t sys32_openat(sys32_context_t context, sys32_int_t fd, const sys32_char_t* pathname, sys32_int_t flags, sys32_mode_t mode)
{
	return static_cast<sys32_long_t>(sys_openat(reinterpret_cast<SystemCall::Context*>(context), fd, pathname, flags, mode));
}

#ifdef _M_X64
// sys64_openat
//
sys64_long_t sys64_openat(sys64_context_t context, sys64_int_t fd, const sys64_char_t* pathname, sys64_int_t flags, sys64_mode_t mode)
{
	return sys_openat(reinterpret_cast<SystemCall::Context*>(context), fd, pathname, flags, mode);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
