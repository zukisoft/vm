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

#include "SystemCallContext.h"
#include "Thread.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_sigaltstack
//
// Sets the alternate stack to use for a thread signal handler
//
// Arguments:
//
//	context		- System call context object
//	newstack	- New stack information to use for signal handlers
//	oldstack	- Receives the previously set stack information

uapi::long_t sys_sigaltstack(const Context* context, const uapi::stack_t* newstack, uapi::stack_t* oldstack)
{
	return -LINUX_ENOSYS;

	//context->Thread->SetSignalAlternateStack(newstack, oldstack);
	//return 0;
}

// sys32_sigaltstack
//
sys32_long_t sys32_sigaltstack(sys32_context_t context, const sys32_stack_t* newstack, sys32_stack_t* oldstack)
{
#ifndef _M_X64
	
	// 32-bit builds should have equivalent definitions for stack_t and sys32_stack_t
	static_assert(sizeof(uapi::stack_t) == sizeof(sys32_stack_t), "uapi::stack_t is not equivalent to sys32_stack_t");
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_sigaltstack, context, reinterpret_cast<const uapi::stack_t*>(newstack), reinterpret_cast<uapi::stack_t*>(oldstack)));

#else

	// 64-bit builds need to convert the input and output structures into the 32-bit versions
	uapi::stack_t convertnew, convertold;
	if(newstack) convertnew = { reinterpret_cast<void*>(newstack->ss_sp), newstack->ss_flags, newstack->ss_size };

	// Invoke the system call using the local conversion structures
	sys32_long_t result = static_cast<sys32_long_t>(SystemCall::Invoke(sys_sigaltstack, context, (newstack) ? &convertnew : nullptr, &convertold));

	// If the caller wanted the old stack information, convert it into the output structure
	if((result == 0) && (oldstack)) *oldstack = { reinterpret_cast<sys32_addr_t>(convertold.ss_sp), convertold.ss_flags, static_cast<sys32_size_t>(convertold.ss_size) };
	return result;

#endif
}

#ifdef _M_X64
// sys64_sigaltstack
//
sys64_long_t sys64_sigaltstack(sys64_context_t context, const sys64_stack_t* newstack, sys64_stack_t* oldstack)
{
	// 64-bit builds should have equivalent definitions for stack_t and sys64_stack_t
	static_assert(sizeof(uapi::stack_t) == sizeof(sys64_stack_t), "uapi::stack_t is not equivalent to sys64_stack_t");
	return SystemCall::Invoke(sys_sigaltstack, context, reinterpret_cast<const uapi::stack_t*>(newstack), reinterpret_cast<uapi::stack_t*>(oldstack));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
