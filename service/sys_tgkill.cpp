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
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_tgkill
//
// Sends a signal to a thread
//
// Arguments:
//
//	context		- System call context object
//	tgid		- Target thread group
//	pid			- Target thread identifier
//	sig			- Signal to send to the thread

uapi::long_t sys_tgkill(const Context* context, uapi::pid_t tgid, uapi::pid_t pid, int sig)
{
	(context);
	(tgid);
	(pid);
	(sig);

	return 0;
}

// sys32_tgkill
//
sys32_long_t sys32_tgkill(sys32_context_t context, sys32_pid_t tgid, sys32_pid_t pid, sys32_int_t sig)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_tgkill, context, tgid, pid, sig));
}

#ifdef _M_X64
// sys64_tgkill
//
sys64_long_t sys64_tgkill(sys64_context_t context, sys64_pid_t tgid, sys64_pid_t pid, sys64_int_t sig)
{
	return SystemCall::Invoke(sys_tgkill, context, tgid, pid, sig);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
