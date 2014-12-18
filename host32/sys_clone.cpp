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
#include <linux\mman.h>

#pragma warning(push, 4)

// g_rpccontext (main.cpp)
//
// RPC context handle
extern sys32_context_t g_rpccontext;

//-----------------------------------------------------------------------------
// sys_clone
//
// Creates a child process or thread
//
// Arguments:
//
//	context		- Pointer to the CONTEXT structure from the exception handler

uapi::long_t sys_clone(PCONTEXT context)
{
	// Create the task state segment for the new process/thread based on the point where the
	// exception handler took over (EIP is already moved forward at this point)
	CONTEXT tss;
	memcpy(&tss, context, sizeof(CONTEXT));							

	// TODO: WORDS ON WHY THIS WORKS (IF IT DOES)

	return sys32_clone(g_rpccontext,				// context
		reinterpret_cast<sys32_uchar_t*>(&tss),		// task_state
		sizeof(CONTEXT),							// task_state_len
		static_cast<sys32_ulong_t>(context->Ebx),	// clone_flags
		static_cast<sys32_addr_t>(context->Ecx),	// child_stack
		static_cast<sys32_addr_t>(context->Edx),	// parent_tidptr
		static_cast<sys32_int_t>(context->Esi),		// tls_val
		static_cast<sys32_addr_t>(context->Edi));	// child_tidptr
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


