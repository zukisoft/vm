//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
// sys_madvise
//
// Provides guidance as to how a range of memory will be used
//
// Arguments:
//
//	context		- System call context object
//	address		- Base address of memory range to advise about
//	length		- Length of the memory range
//	advice		- Advice from the calling process about the region's usage

uapi::long_t sys_madvise(const Context* context, void* address, size_t length, int advice)
{
	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(address);
	UNREFERENCED_PARAMETER(length);
	UNREFERENCED_PARAMETER(advice);
	
	return -LINUX_ENOSYS;

	//// This system call is currently a no-op
	//return 0;
}

// sys32_madvise
//
sys32_long_t sys32_madvise(sys32_context_t context, sys32_addr_t addr, sys32_size_t length, sys32_int_t advice)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_madvise, context, reinterpret_cast<void*>(addr), length, advice));
}

#ifdef _M_X64
// sys64_madvise
//
sys64_long_t sys64_madvise(sys64_context_t context, sys64_addr_t addr, sys64_size_t length, sys64_int_t advice)
{
	return SystemCall::Invoke(sys_madvise, context, reinterpret_cast<void*>(addr), length, advice);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
