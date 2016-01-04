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

#include "SystemCallContext.h"
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_brk
//
// Sets the program break address for a process
//
// Arguments:
//
//	context			- System call context object
//	brk				- Requested new program break address

uapi::long_t sys_brk(const Context* context, void* brk)
{
	return -LINUX_ENOSYS;
	//// Set the process program break and return the updated address
	//return reinterpret_cast<uapi::long_t>(context->Process->SetProgramBreak(brk));
}

// sys32_brk
//
sys32_long_t sys32_brk(sys32_context_t context, sys32_addr_t brk)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_brk, context, reinterpret_cast<void*>(brk)));
}

#ifdef _M_X64
// sys64_brk
//
sys64_long_t sys64_brk(sys64_context_t context, sys64_addr_t brk)
{
	return SystemCall::Invoke(sys_brk, context, reinterpret_cast<void*>(brk));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
