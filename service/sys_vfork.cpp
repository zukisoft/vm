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

#pragma warning(push, 4)

// sys_clone.cpp
//
uapi::long_t sys_clone(const Context* context, void* taskstate, size_t taskstatelen, uint32_t flags, uapi::pid_t* ptid, uapi::pid_t* ctid, uapi::user_desc32* tls_val);

//-----------------------------------------------------------------------------
// sys_vfork
//
// Creates a child process and blocks the parent
//
// Arguments:
//
//	context			- System call context object
//	taskstate		- Child task startup information
//	taskstatelen	- Length of the child task startup information

uapi::long_t sys_vfork(const Context* context, void* taskstate, size_t taskstatelen)
{
	return -LINUX_ENOSYS;

	//// sys_vfork is equivalent to sys_clone(CLONE_VFORK | CLONE_VM | SIGCHLD)
	//return sys_clone(context, taskstate, taskstatelen, LINUX_CLONE_VFORK | LINUX_CLONE_VM | LINUX_SIGCHLD, nullptr, nullptr, nullptr);
}

// sys32_vfork
//
sys32_long_t sys32_vfork(sys32_context_t context, sys32_task_t* taskstate)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_vfork, context, taskstate, sizeof(sys32_task_t)));
}

#ifdef _M_X64
// sys64_vfork
//
sys64_long_t sys64_vfork(sys64_context_t context, sys64_task_state_t* taskstate)
{
	return SystemCall::Invoke(sys_vfork, context, taskstate, sizeof(sys64_task_state_t));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
