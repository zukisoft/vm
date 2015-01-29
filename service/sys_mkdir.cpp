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

// sys_mkdirat.cpp
//
__int3264 sys_mkdirat(const ContextHandle* context, int fd, const uapi::char_t* pathname, uapi::mode_t mode);

//-----------------------------------------------------------------------------
// sys_mkdir
//
// Creates a directory file system object
//
// Arguments:
//
//	context		- SystemCall context object
//	pathname	- Path to the new directory to be created
//	mode		- Mode flags to assign to the new directory object

__int3264 sys_mkdir(const ContextHandle* context, const uapi::char_t* pathname, uapi::mode_t mode)
{
	// sys_mkdir() is equivalent to sys_mkdirat(AT_FDCWD)
	return sys_mkdirat(context, LINUX_AT_FDCWD, pathname, mode);
}

// sys32_mkdir
//
sys32_long_t sys32_mkdir(sys32_context_t context, const sys32_char_t* pathname, sys32_mode_t mode)
{
	return static_cast<sys32_long_t>(sys_mkdir(reinterpret_cast<ContextHandle*>(context), pathname, mode));
}

#ifdef _M_X64
// sys64_mkdir
//
sys64_long_t sys64_mkdir(sys64_context_t context, const sys64_char_t* pathname, sys64_mode_t mode)
{
	return sys_mkdir(reinterpret_cast<ContextHandle*>(context), pathname, mode);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
