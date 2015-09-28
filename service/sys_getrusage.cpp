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

#include "SystemCallContext.h"
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_getrusage
//
// Gets accounting information for a process or thread
//
// Arguments:
//
//	context		- System call context object
//	who			- Flag indicating what accounting information to get
//	rusage		- Receives the accounting information data

uapi::long_t sys_getrusage(const Context* context, int who, uapi::rusage* rusage)
{
	return -LINUX_ENOSYS;

	//switch(who) {

	//	// RUSAGE_SELF, RUSAGE_CHILDREN --> Process
	//	case LINUX_RUSAGE_SELF:
	//	case LINUX_RUSAGE_CHILDREN:
	//		context->Process->GetResourceUsage(who, rusage);
	//		break;

	//	// RUSAGE_THREAD --> Thread
	//	case LINUX_RUSAGE_THREAD:
	//		context->Thread->GetResourceUsage(who, rusage);
	//		break;

	//	// Anything else --> EINVAL
	//	default: throw LinuxException(LINUX_EINVAL);
	//}

	//return 0;
}

// sys32_getrusage
//
sys32_long_t sys32_getrusage(sys32_context_t context, sys32_int_t who, linux_rusage32* rusage)
{
	uapi::rusage			usage;				// Generic uapi::rusage structure

	// Invoke the generic version of the system call, passing in the generic uapi::rusage if applicable
	sys32_long_t result = static_cast<sys32_long_t>(SystemCall::Invoke(sys_getrusage, context, who, &usage));

	// Convert the data from the generic rusage structure into the 32-bit linux_rusage32 structure
	if(result >= 0) {

		rusage->ru_utime.tv_sec		= static_cast<int32_t>(usage.ru_utime.tv_sec);
		rusage->ru_utime.tv_usec	= static_cast<int32_t>(usage.ru_utime.tv_usec);
		rusage->ru_systime.tv_sec	= static_cast<int32_t>(usage.ru_systime.tv_sec);
		rusage->ru_systime.tv_usec	= static_cast<int32_t>(usage.ru_systime.tv_usec);
		rusage->ru_maxrss			= static_cast<int32_t>(usage.ru_maxrss);
		rusage->ru_ixrss			= static_cast<int32_t>(usage.ru_ixrss);
		rusage->ru_idrss			= static_cast<int32_t>(usage.ru_idrss);
		rusage->ru_isrss			= static_cast<int32_t>(usage.ru_isrss);
		rusage->ru_minflt			= static_cast<int32_t>(usage.ru_minflt);
		rusage->ru_majflt			= static_cast<int32_t>(usage.ru_majflt);
		rusage->ru_nswap			= static_cast<int32_t>(usage.ru_nswap);
		rusage->ru_inblock			= static_cast<int32_t>(usage.ru_inblock);
		rusage->ru_oublock			= static_cast<int32_t>(usage.ru_oublock);
		rusage->ru_msgsnd			= static_cast<int32_t>(usage.ru_msgsnd);
		rusage->ru_msgrcv			= static_cast<int32_t>(usage.ru_msgrcv);
		rusage->ru_nsignals			= static_cast<int32_t>(usage.ru_nsignals);
		rusage->ru_nvcsw			= static_cast<int32_t>(usage.ru_nvcsw);
		rusage->ru_nivcsw			= static_cast<int32_t>(usage.ru_nivcsw);
	}

	return result;
}

#ifdef _M_X64
// sys64_getrusage
//
sys64_long_t sys64_getrusage(sys64_context_t context, sys64_int_t who, linux_rusage64* rusage)
{
	return SystemCall::Invoke(sys_wait4, context, who, rusage);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
