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

//-----------------------------------------------------------------------------
// sys_clone
//
// Creates a child process or thread
//
// Arguments:
//
//	context			- SystemCall context object
//	tss				- Child task state segment (CONTEXT blob)
//	tsslen			- Length of the child task state segment
//	flags			- Clone operation flags
//	newstack		- New child stack address
//	ptid			- Address to store the new child pid_t (in parent and child)
//	ctid			- Address to store the new child pit_t (in child only)
//	tls				- Ignored (new thread local storage descriptor)

__int3264 sys_clone(const SystemCall::Context* context, void* tss, size_t tsslen, uint32_t flags, void* newstack, uapi::pid_t* ptid, uapi::pid_t* ctid, int tls)
{
	_ASSERTE(context);
	UNREFERENCED_PARAMETER(tls);				// Linux doesn't appear to ever use the tls argument

	(newstack);		// <---- todo; need to know how this works and if I should send it in the TSS or as an argument
	(ptid);
	(ctid);

	// NOTE: GLIBC sends in CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD for flags when fork(3) is called
	// (0x01200011)

	try { 

		SystemCall::Impersonation impersonation;

		auto parent = context->Process;
		auto child = context->VirtualMachine->CloneProcess(parent, flags, tss, tsslen);

		// Write the new process/thread identifier into the requested locations for the parent and child
		// todo: these are enabled via clone_flags
		//uapi::pid_t newpid = child->ProcessId;
		//if(ptid) parent->WriteMemory(ptid, &newpid, sizeof(uapi::pid_t));
		//if(ptid) child->WriteMemory(ptid, &newpid, sizeof(uapi::pid_t));
		//if(ctid) child->WriteMemory(ctid, &newpid, sizeof(uapi::pid_t));

		return child->ProcessId;
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_clone
//
sys32_long_t sys32_clone(sys32_context_t context, sys32_uchar_t* task_state, sys32_size_t task_state_len, sys32_ulong_t clone_flags, sys32_addr_t child_stack, sys32_addr_t parent_tidptr, sys32_int_t tls_val, sys32_addr_t child_tidptr)
{
	// Note that the parameter order for the x86 system call differs from the standard system call, ctid and tls are swapped
	return static_cast<sys32_long_t>(sys_clone(reinterpret_cast<SystemCall::Context*>(context), task_state, task_state_len, clone_flags, 
		reinterpret_cast<void*>(child_stack), reinterpret_cast<uapi::pid_t*>(parent_tidptr), reinterpret_cast<uapi::pid_t*>(child_tidptr), tls_val));
}

#ifdef _M_X64
// sys64_clone
//
sys64_long_t sys64_clone(sys64_context_t context, sys64_registers* registers, sys64_uint_t calling_thread_id, sys64_ulong_t clone_flags, sys64_addr_t newsp, sys64_addr_t parent_tidptr, sys64_addr_t child_tidptr, sys64_int_t tls_val)
{
	//return sys_clone(reinterpret_cast<SystemCall::Context*>(context), static_cast<uint32_t>(clone_flags), reinterpret_cast<void*>(newsp), 
	//	reinterpret_cast<uapi::pid_t*>(parent_tidptr), reinterpret_cast<uapi::pid_t*>(child_tidptr), tls_val);
	(context);
	(clone_flags);
	(newsp);
	(parent_tidptr);
	(child_tidptr);
	(tls_val);

	return -LINUX_ENOSYS;;
}

#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
