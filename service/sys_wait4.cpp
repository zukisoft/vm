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
// sys_wait4
//
// Waits for a process to change state
//
// Arguments:
//
//	context		- System call context object
//	pid			- PID to wait upon
//	status		- Optionally receives PID status information
//	options		- Wait operation options
//	rusage		- Optionally receives child accounting information

uapi::long_t sys_wait4(const Context* context, uapi::pid_t pid, int* status, int options, uapi::rusage* rusage)
{
	uapi::idtype_t			type;				// Wait identifier type for Process::WaitChild
	uapi::pid_t				id;					// Wait identifier for Process::WaitChild

	// Verify the validity of the options mask for this operation, they differ between the wait family
	// of system calls but Process::WaitChild accepts a superset of them, so it has to be checked here
	if(options & ~(LINUX_WNOHANG | LINUX_WUNTRACED | LINUX_WCONTINUED | LINUX__WNOTHREAD | LINUX__WCLONE | LINUX__WALL)) return -LINUX_EINVAL;

	// Pull out a reference to the context Process object instance
	auto process = context->Process;

	// pid < -1 - Absolute value indicates a specific process group identifier
	if(pid < -1) { type = LINUX_P_PGID; id = -pid; }

	// pid == -1 - Indicates wait for any child process
	else if(pid == -1) { type = LINUX_P_ALL; id = -1; }

	// pid == 0 - Indicates wait for children in the same process group
	else if(pid == 0) { _ASSERTE(false); type = LINUX_P_PGID; /* id = process->ProcessGroupId; */ }

	// pid > 0 - Indicates a specific child process identifier
	else { type = LINUX_P_PID; id = pid; }

	// Execute the wait operation and return the signaled child pid, automatically applying WEXITED
	return process->WaitChild(type, id, status, options | LINUX_WEXITED, nullptr, rusage);
}

// sys32_wait4
//
sys32_long_t sys32_wait4(sys32_context_t context, sys32_pid_t pid, sys32_int_t* status, sys32_int_t options, linux_rusage32* rusage)
{
	uapi::rusage			usage;				// Child accounting information

	// Invoke the generic version of the system call using the local structure
	sys32_long_t result = static_cast<sys32_long_t>(SystemCall::Invoke(sys_wait4, context, pid, status, options, &usage));

	// If sys_wait4 was successful, convert the data from the generic structure into the compatible one
	if((result >= 0) && (rusage != nullptr)) {

		// TODO - convert stuff here; just zero it out for now I don't even know if
		// the rusage structures are the right size yet
		memset(rusage, 0, sizeof(linux_rusage32));
	}

	return result;
}

#ifdef _M_X64
// sys64_wait4
//
sys64_long_t sys64_wait4(sys64_context_t context, sys64_pid_t pid, sys64_int_t* status, sys64_int_t options, linux_rusage64* rusage)
{
	return SystemCall::Invoke(sys_wait4, context, pid, status, options, rusage);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
