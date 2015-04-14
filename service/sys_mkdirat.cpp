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
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_mkdirat
//
// Creates a directory relative to an existing directory file descriptor
//
// Arguments:
//
//	context		- System call context object
//	dirfd		- Previously opened directory object file descriptor
//	pathname	- Relative path for the directory to create
//	mode		- Mode flags to assign when creating the directory

uapi::long_t sys_mkdirat(const Context* context, int dirfd, const uapi::char_t* pathname, uapi::mode_t mode)
{
	// Determine if an absolute or relative pathname has been provided
	bool absolute = ((pathname) && (pathname[0] == '/'));

	// Determine the base alias from which to resolve the path
	std::shared_ptr<FileSystem::Alias> base = absolute ? context->Process->RootDirectory : 
		((dirfd == LINUX_AT_FDCWD) ? context->Process->WorkingDirectory : context->Process->Handle[dirfd]->Alias);

	// Apply the process' current umask to the provided creation mode flags
	mode &= ~context->Process->FileCreationModeMask;

	// Attempt to create the directory object relative from the base alias
	FileSystem::CreateDirectory(context->Process->RootDirectory, base, pathname, mode);

	return 0;
}

// sys32_mkdirat
//
sys32_long_t sys32_mkdirat(sys32_context_t context, sys32_int_t dirfd, const sys32_char_t* pathname, sys32_mode_t mode)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_mkdirat, context, dirfd, pathname, mode));
}

#ifdef _M_X64
// sys64_mkdirat
//
sys64_long_t sys64_mkdirat(sys64_context_t context, sys64_int_t dirfd, const sys64_char_t* pathname, sys64_mode_t mode)
{
	return SystemCall::Invoke(sys_mkdirat, context, dirfd, pathname, mode);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
