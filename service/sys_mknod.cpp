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

// sys_mknodat.cpp
//
__int3264 sys_mknodat(const SystemCall::Context* context, int fd, const uapi::char_t* pathname, uapi::mode_t mode, uapi::dev_t device);

//-----------------------------------------------------------------------------
// sys_mknod
//
// Creates a special node
//
// Arguments:
//
//	context		- SystemCall context object
//	pathname	- Path to the new node to be created
//	mode		- Mode flags to assign to the new directory object
//	device		- Device identifier when creating a device node

__int3264 sys_mknod(const SystemCall::Context* context, const uapi::char_t* pathname, uapi::mode_t mode, uapi::dev_t device)
{
	// sys_mknod() is equivalent to sys_mknodat(AT_FDCWD)
	return sys_mknodat(context, LINUX_AT_FDCWD, pathname, mode, device);
}

// sys32_mknod
//
sys32_long_t sys32_mknod(sys32_context_t context, const sys32_char_t* pathname, sys32_mode_t mode, sys32_dev_t device)
{
	return static_cast<sys32_long_t>(sys_mknod(reinterpret_cast<SystemCall::Context*>(context), pathname, mode, device));
}

#ifdef _M_X64
// sys64_mknod
//
sys64_long_t sys64_mknod(sys64_context_t context, const sys64_char_t* pathname, sys64_mode_t mode, sys64_dev_t device)
{
	return sys_mknod(reinterpret_cast<SystemCall::Context*>(context), pathname, mode, device);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
