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

// sys_fstatat64.cpp
//
uapi::long_t sys_fstatat64(const Context* context, int fd, const uapi::char_t* pathname, linux_stat3264* buf, int flags);

//-----------------------------------------------------------------------------
// sys_lstat64
//
// Get information and statistics about a file system object
//
// Arguments:
//
//	context		- System call context object
//	pathname	- Pathname to the file system object
//	stats		- Output structure

uapi::long_t sys_lstat64(const Context* context, const uapi::char_t* pathname, linux_stat3264* buf)
{
	return -LINUX_ENOSYS;

	//// sys_lstat64 is equivalent to sys_fstatat64(LINUX_AT_FDCWD, LINUX_AT_SYMLINK_NOFOLLOW)
	//return sys_fstatat64(context, LINUX_AT_FDCWD, pathname, buf, LINUX_AT_SYMLINK_NOFOLLOW);
}

// sys32_lstat64
//
sys32_long_t sys32_lstat64(sys32_context_t context, const sys32_char_t* pathname, linux_stat3264* buf)
{
	return SystemCall::Invoke(sys_lstat64, context, pathname, buf);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
