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
//	clone_flags		- Operation flags
//	newsp			- New child process/thread stack address
//	parent_tidptr	- Address to store the new child pid_t (in parent and child)
//	child_tidptr	- Address to store the new child pit_t (in child only)
//	tls_val			- Ignored; new thread local storage descriptor 

__int3264 sys_clone(const SystemCall::Context* context, uint32_t clone_flags, void* newsp, uapi::pid_t* parent_tidptr, uapi::pid_t* child_tidptr, int tls_val)
{
	_ASSERTE(context);
	UNREFERENCED_PARAMETER(tls_val);

	(newsp);
	(parent_tidptr);
	(child_tidptr);

	// Linux doesn't seem to use tls_val in clone(2) at all, it's just ignored completely

	// GLIBC sends in CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD for clone_flags when fork(3) is called
	// (0x01200011)

	// if newsp is null, the original process stack should become copy-on-write (not possible right now,
	// will start by just copying everything into a new process and go from there I guess), so in that
	// case the stack pointer should match the parent's stack pointer?  How on Earth am I going to make
	// that work? Gonna need to get the context for the calling thread, which can't be done while the
	// thread is running.  Should be fun :)

	try { 

		SystemCall::Impersonation impersonation;

		auto child = context->VirtualMachine->CloneProcess(context->Process, clone_flags);
		if(child == nullptr) throw LinuxException(LINUX_ENOSYS);	// <--- TODO: Proper exception

		auto newpid = child->ProcessId;

		if(parent_tidptr != 0) { (newpid); /* WRITE NEW PID HERE? */ }
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return 0;
}

// sys32_clone
//
sys32_long_t sys32_clone(sys32_context_t context, sys32_ulong_t clone_flags, sys32_addr_t newsp, sys32_addr_t parent_tidptr, sys32_int_t tls_val, sys32_addr_t child_tidptr)
{
	// Note that the parameter order for the x86 system call differs from the standard system call, child_tidptr and tls_val get swapped here
	return static_cast<sys32_long_t>(sys_clone(reinterpret_cast<SystemCall::Context*>(context), clone_flags, reinterpret_cast<void*>(newsp), 
		reinterpret_cast<uapi::pid_t*>(parent_tidptr), reinterpret_cast<uapi::pid_t*>(child_tidptr), tls_val));
}

#ifdef _M_X64
// sys64_clone
//
sys64_long_t sys64_clone(sys64_context_t context, sys64_ulong_t clone_flags, sys64_addr_t newsp, sys64_addr_t parent_tidptr, sys64_addr_t child_tidptr, sys64_int_t tls_val)
{
	return sys_clone(reinterpret_cast<SystemCall::Context*>(context), static_cast<uint32_t>(clone_flags), reinterpret_cast<void*>(newsp), 
		reinterpret_cast<uapi::pid_t*>(parent_tidptr), reinterpret_cast<uapi::pid_t*>(child_tidptr), tls_val);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
