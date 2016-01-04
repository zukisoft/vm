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

// sys_wait4.cpp
//
uapi::long_t sys_wait4(const Context* context, uapi::pid_t pid, int* status, int options, uapi::rusage* rusage);

//-----------------------------------------------------------------------------
// sys_waitpid
//
// Waits for a process to change state
//
// Arguments:
//
//	context		- System call context object
//	pid			- PID to wait upon
//	status		- Optionally receives PID status information
//	options		- Wait operation options

uapi::long_t sys_waitpid(const Context* context, uapi::pid_t pid, int* status, int options)
{
	return -LINUX_ENOSYS;

	//// sys_waitpid is equivalent to sys_wait4(pid, status, options, nullptr)
	//return sys_wait4(context, pid, status, options, nullptr);
}

// sys32_waitpid
//
sys32_long_t sys32_waitpid(sys32_context_t context, sys32_pid_t pid, sys32_int_t* status, sys32_int_t options)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_waitpid, context, pid, status, options));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
