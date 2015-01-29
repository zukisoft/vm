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
#include "ContextHandle.h"

#pragma warning(push, 4)

// sys_faccessat.cpp
//
uapi::long_t sys_faccessat(const ContextHandle* context, int dirfd, const uapi::char_t* pathname, uapi::mode_t mode, int flags);

//-----------------------------------------------------------------------------
// sys_access
//
// Checks permissions to a file system object
//
// Arguments:
//
//	context		- SystemCall context object
//	pathname	- Relative path for the file system object to check
//	mode		- Accessibility check mask (F_OK, R_OK, etc)

uapi::long_t sys_access(const ContextHandle* context, const uapi::char_t* pathname, uapi::mode_t mode)
{
	// sys_access() is equivalent to sys_faccessat(AT_FDCWD)
	return sys_faccessat(context, LINUX_AT_FDCWD, pathname, mode, 0);
}

// sys32_access
//
sys32_long_t sys32_access(sys32_context_t context, const sys32_char_t* pathname, sys32_mode_t mode)
{
	return static_cast<sys32_long_t>(sys_access(reinterpret_cast<ContextHandle*>(context), pathname, mode));
}

#ifdef _M_X64
// sys64_access
//
sys64_long_t sys64_access(sys64_context_t context, const sys64_char_t* pathname, sys64_mode_t mode)
{
	return sys_access(reinterpret_cast<ContextHandle*>(context), pathname, mode);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
