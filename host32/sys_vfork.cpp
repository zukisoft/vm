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
#include <linux\ldt.h>

#pragma warning(push, 4)

// t_rpccontext (main.cpp)
//
// RPC context handle for the current thread
extern __declspec(thread) sys32_context_t t_rpccontext;

// t_gs (emulator.cpp)
//
// Emulated GS register value
extern __declspec(thread) uint16_t t_gs;

//-----------------------------------------------------------------------------
// sys_vfork
//
// Creates a child process and blocks the parent
//
// Arguments:
//
//	context		- Pointer to the CONTEXT structure from the exception handler

uapi::long_t sys_vfork(PCONTEXT context)
{
	zero_init<sys32_task_t>			taskstate;		// Child task state data

	// The result of sys_vfork in the child process should be zero, set EAX
	taskstate.eax = 0;

	// Set the other general-purpose registers based on their current values
	taskstate.ebx = context->Ebx;
	taskstate.ecx = context->Ecx;
	taskstate.edx = context->Edx;
	taskstate.edi = context->Edi;
	taskstate.esi = context->Esi;
	taskstate.eip = context->Eip;
	taskstate.ebp = context->Ebp;
	taskstate.esp = context->Esp;

	// Copy this thread's current emulated GS register value
	taskstate.gs = t_gs;

	// Invoke sys_vfork with the generated startup information for the new process
	return sys32_vfork(t_rpccontext, &taskstate);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


