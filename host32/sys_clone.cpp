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
#include <linux\ldt.h>

#pragma warning(push, 4)

// g_rpccontext (main.cpp)
//
// RPC context handle
extern sys32_context_t g_rpccontext;

// t_gs (emulator.cpp)
//
// Emulated GS register value
extern __declspec(thread) uint32_t t_gs;

// t_ldt (emulator.cpp)
//
// Thread-local LDT
extern __declspec(thread) sys32_ldt_t t_ldt;

// AllocateLDTEntry (emulator.cpp)
//
sys32_ldt_entry_t* AllocateLDTEntry(sys32_ldt_t* ldt, sys32_ldt_entry_t* entry);

// sys32_ldt_entry_t and uapi::user_desc must be the same size
//
static_assert(sizeof(sys32_ldt_entry_t) == sizeof(uapi::user_desc), "sys32_ldt_entry_t is not the same size as uapi::user_desc");

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
	zero_init<sys32_task_state_t>		taskstate;		// Child task state data

	// Cast out the arguments to sys_clone
	sys32_ulong_t		clone_flags		= static_cast<sys32_ulong_t>(context->Ebx);	
	void*				child_stack		= reinterpret_cast<void*>(context->Ecx);
	uapi::pid_t*		parent_tidptr	= reinterpret_cast<uapi::pid_t*>(context->Edx);
	uapi::user_desc*	tls_val			= reinterpret_cast<uapi::user_desc*>(context->Esi);
	uapi::pid_t*		child_tidptr	= reinterpret_cast<uapi::pid_t*>(context->Edi);

	// The result of sys_clone in the child process/thread should be zero, set EAX
	taskstate.eax = 0;

	// Set the other general-purpose registers based on their current values
	taskstate.ebx = context->Ebx;
	taskstate.ecx = context->Ecx;
	taskstate.edx = context->Edx;
	taskstate.edi = context->Edi;
	taskstate.esi = context->Esi;
	taskstate.eip = context->Eip;

	// Set the frame and stack pointers explicitly if requested, otherwise use this thread's registers
	// TODO: What should EBP be set to if there is a new stack pointer?
	taskstate.ebp = (child_stack) ? reinterpret_cast<sys32_addr_t>(child_stack) : context->Ebp;
	taskstate.esp = (child_stack) ? reinterpret_cast<sys32_addr_t>(child_stack) : context->Esp;

	// Copy this thread's current emulated GS register value
	taskstate.gs = t_gs;

	// Copy this thread's local descriptor table into the startup information
	memcpy(&taskstate.ldt, &t_ldt, sizeof(sys32_ldt_t));

	// If a new TLS slot is to be allocated in the cloned process, allocate it in the startup
	// information, but do not actually modify this thread's emulated LDT
	if(tls_val) AllocateLDTEntry(&taskstate.ldt, reinterpret_cast<sys32_ldt_entry_t*>(tls_val));

	// Invoke sys_clone with the generated startup information for the new process/thread
	return sys32_clone(g_rpccontext, &taskstate, clone_flags, reinterpret_cast<sys32_addr_t>(parent_tidptr), 
		reinterpret_cast<sys32_addr_t>(child_tidptr));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


