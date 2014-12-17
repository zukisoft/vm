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
	sys32_registers		registers;				// Child process/thread registers

	// Initialize the child process/thread registers based on the provided CONTEXT,
	// skipping over the INT 80H instruction currently being processed

	registers.EAX = context->Eax;
	registers.EBX = context->Ebx;
	registers.ECX = context->Ecx;
	registers.EDX = context->Edx;
	registers.EDI = context->Edi;
	registers.ESI = context->Esi;
	registers.EBP = context->Ebp;
	registers.EIP = context->Eip + 2;			// INT 80H [0xCD, 0x80]
	registers.ESP = context->Esp;

	// sys32_clone takes 2 additional arguments over the clone(2) system call, it needs to know what the
	// state of the registers should be in the cloned thread/process as well as the thread id that's
	// going to be cloned within this process (which is this thread). The thread that invoked sys32_clone() 
	// comes back here like it normally would, whereas the newly created process/thread will jump to the 
	// location specified by the registers, bypassing the emulator/exception handler layer.  This is why 
	// the instruction pointer had to be incremented beyond the INT 80H call in the passed register set

	return sys32_clone(g_rpccontext,				// context
		&registers,									// registers
		GetCurrentThreadId(),						// calling_thread_id
		static_cast<sys32_ulong_t>(context->Ebx),	// clone_flags
		static_cast<sys32_addr_t>(context->Ecx),	// newsp
		static_cast<sys32_addr_t>(context->Edx),	// parent_tidptr
		static_cast<sys32_int_t>(context->Esi),		// tls_val
		static_cast<sys32_addr_t>(context->Edi));	// child_tidptr
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


