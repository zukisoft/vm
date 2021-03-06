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
// sys_clone
//
// Creates a child process or thread
//
// Arguments:
//
//	context			- System call context object
//	taskstate		- Child task startup information
//	taskstatelen	- Length of the child task startup information
//	flags			- Clone operation flags
//	ptid			- Address to store the new child pid_t (in parent and child)
//	ctid			- Address to store the new child pit_t (in child only)
//	tls_val			- Optional new thread-local storage descriptor for the child

uapi::long_t sys_clone(const Context* context, void* taskstate, size_t taskstatelen, uint32_t flags, uapi::pid_t* ptid, uapi::pid_t* ctid, uapi::user_desc32* tls_val)
{
	return -LINUX_ENOSYS;

	//// NOTE: GLIBC sends in CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD for flags when fork(3) is called
	//// (0x01200011)

	//auto parent = context->Process;
	//auto child = context->_VmOld->CloneProcess(parent, flags, taskstate, taskstatelen);

	////
	//// TODO: CHILD IS RUNNING; EVERYTHING BELOW IS A RACE CONDITION
	////

	//// Acquire the process identifier from the newly created process object instance
	//uapi::pid_t newpid = child->ProcessId;

	//// CLONE_PARENT_SETTID
	////
	//// Write the new process/thread identifier to the specified location in both the parent and child
	//if((flags & LINUX_CLONE_PARENT_SETTID) && ptid) {

	//	parent->WriteMemory(ptid, &newpid, sizeof(uapi::pid_t));
	//	child->WriteMemory(ptid, &newpid, sizeof(uapi::pid_t));
	//}

	//// CLONE_CHILD_SETTID
	////
	//// Write the new process/thread identifier to the specified location in the child's address space
	//if((flags & LINUX_CLONE_CHILD_SETTID) && ctid) child->WriteMemory(ctid, &newpid, sizeof(uapi::pid_t));

	//// CLONE_CHILD_CLEARTID
	////
	//// Sets a pointer to the thread id in the child process.  See set_tid_address(2) for more details.
	//auto thread = child->Thread[newpid];
	//if((flags & LINUX_CLONE_CHILD_CLEARTID) && ctid) thread->ClearThreadIdOnExit = reinterpret_cast<void*>(ctid);

	//// CLONE_SETTLS
	////
	//// Indicates that a new thread-local storage descriptor should be set for the child process/thread
	//if((flags & LINUX_CLONE_SETTLS) && tls_val) child->SetLocalDescriptor(tls_val);

	//// Start the child process -- NOTE thread does not exist yet if done here, CLEARTID above will crash
	////child->Start();

	//// The calling process gets the new PID as the result
	//return static_cast<uapi::long_t>(newpid);
}

// sys32_clone
//
sys32_long_t sys32_clone(sys32_context_t context, sys32_task_t* taskstate, sys32_ulong_t clone_flags, sys32_addr_t parent_tidptr, sys32_addr_t child_tidptr, linux_user_desc32* tls_val)
{
	// Note that the parameter order for the x86 system call differs from the standard system call, ctid and tls are swapped
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_clone, context, taskstate, sizeof(sys32_task_t), clone_flags, 
		reinterpret_cast<uapi::pid_t*>(parent_tidptr), reinterpret_cast<uapi::pid_t*>(child_tidptr), reinterpret_cast<uapi::user_desc32*>(tls_val)));
}

#ifdef _M_X64
// sys64_clone
//
sys64_long_t sys64_clone(sys64_context_t context, sys64_task_state_t* taskstate, sys64_ulong_t clone_flags, sys64_addr_t parent_tidptr, sys64_addr_t child_tidptr)
{
	return SystemCall::Invoke(sys_clone, context, taskstate, sizeof(sys64_task_state_t), static_cast<uint32_t>(clone_flags),
		reinterpret_cast<uapi::pid_t*>(parent_tidptr), reinterpret_cast<uapi::pid_t*>(child_tidptr));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
