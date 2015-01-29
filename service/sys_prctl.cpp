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

//-----------------------------------------------------------------------------
// sys_prctl
//
// Perform process-specific operations
//
// Arguments:
//
//	context		- SystemCall context object
//	option		- Operation to be performed
//	arg2		- Optional operation-specific argument
//	arg3		- Optional operation-specific argument
//	arg4		- Optional operation-specific argument
//	arg5		- Optional operation-specific argument

__int3264 sys_prctl(const ContextHandle* context, int option, uapi::ulong_t arg2, uapi::ulong_t arg3, uapi::ulong_t arg4, uapi::ulong_t arg5)
{
	_ASSERTE(context);

	(option);
	(arg2);
	(arg3);
	(arg4);
	(arg5);

	return -LINUX_ENOSYS;
}

// sys32_prctl
//
sys32_long_t sys32_prctl(sys32_context_t context, sys32_int_t option, sys32_ulong_t arg2, sys32_ulong_t arg3, sys32_ulong_t arg4, sys32_ulong_t arg5)
{
	return static_cast<sys32_long_t>(sys_prctl(reinterpret_cast<ContextHandle*>(context), option, arg2, arg3, arg4, arg5));
}

#ifdef _M_X64
// sys64_prctl
//
sys64_long_t sys64_prctl(sys64_context_t context, sys64_int_t option, sys64_ulong_t arg2, sys64_ulong_t arg3, sys64_ulong_t arg4, sys64_ulong_t arg5)
{
	return sys_prctl(reinterpret_cast<ContextHandle*>(context), option, arg2, arg3, arg4, arg5);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
