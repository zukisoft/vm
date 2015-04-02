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

#pragma warning(push, 4)

// t_exittask (main.cpp)
//
// Thread-local task state information to restore original thread
extern __declspec(thread) sys32_task_t t_exittask;

// t_rpccontext (main.cpp)
//
// RPC context handle for the current thread
extern __declspec(thread) sys32_context_t t_rpccontext;

//-----------------------------------------------------------------------------
// sys_exit
//
// Terminates the calling thread
//
// Arguments:
//
//	context		- Pointer to the CONTEXT structure from the exception handler

uapi::long_t sys_exit(PCONTEXT context)
{
	// Cast out the arguments to sys_exit
	sys32_ulong_t status = static_cast<sys32_ulong_t>(context->Ebx);

	// Convert the exit code into a process exit code, lower byte remains zero
	// to indicate that the process terminated normally rather than by signal
	status = ((status & 0xFF) << 8);

	// Restore the saved task information, this will cause the thread to jump back
	// to the original point where it was forked when the CONTEXT is reapplied, the
	// sys32_exit() system call will be executed from there
	context->Eax = status;
	context->Ebx = t_exittask.ebx;
	context->Ecx = t_exittask.ecx;
	context->Edx = t_exittask.edx;
	context->Edi = t_exittask.edi;
	context->Esi = t_exittask.esi;
	context->Eip = t_exittask.eip;
	context->Ebp = t_exittask.ebp;
	context->Esp = t_exittask.esp;

	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


