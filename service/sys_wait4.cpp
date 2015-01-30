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
	int					exitstatus;

	(status);
	(options);
	(rusage);

	uapi::long_t result = static_cast<uapi::pid_t>(context->Process->WaitChild_TEST(pid, &exitstatus));
	if(status) *status = exitstatus;
	return result;
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
