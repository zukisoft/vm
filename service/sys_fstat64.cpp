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
// sys_fstat64
//
// Get information and statistics about a file system object
//
// Arguments:
//
//	context		- System call context object
//	fd			- File descriptor from which to retrieve the stats
//	stats		- Output structure

uapi::long_t sys_fstat64(const Context* context, int fd, linux_stat3264* buf)
{
	return -LINUX_ENOSYS;

	//// sys_fstat64 is equivalent to sys_fstatat64(fd, LINUX_AT_EMPTY_PATH)
	//return sys_fstatat64(context, fd, "", buf, LINUX_AT_EMPTY_PATH);
}

// sys32_fstat64
//
sys32_long_t sys32_fstat64(sys32_context_t context, sys32_int_t fd, linux_stat3264* buf)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_fstat64, context, fd, buf));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
