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
// sys_fcntl
//
// Manipulates an open file descriptor
//
// Arguments:
//
//	context		- SystemCall context object
//	fd			- Open file descriptor to be manipulated
//	cmd			- Operation command code
//	arg			- Optional argument for the specified command code

__int3264 sys_fcntl(const SystemCall::Context* context, int fd, int cmd, void* arg)
{
	_ASSERTE(context);
	(arg);

	try { 		
		
		SystemCall::Impersonation impersonation;

		auto handle = context->Process->GetHandle(fd);

		// Commands are listed in the order described in the man page, not numerically
		switch(cmd) {

			//
			// FILE DESCRIPTOR DUPLICATION
			//

			// F_DUPFD - Duplicate the handle using the original flags
			case LINUX_F_DUPFD:	
				return context->Process->AddHandle(handle->Duplicate(handle->Flags));

			// F_DUPFD_CLOEXEC - Duplicate the handle with O_CLOEXEC specified as well
			case LINUX_F_DUPFD_CLOEXEC:
				return context->Process->AddHandle(handle->Duplicate(handle->Flags | LINUX_O_CLOEXEC));

			//
			// FILE DESCRIPTOR FLAGS
			//

			// F_GETFD - Get the file descriptor flags
			// case LINUX_F_GETFD:
				// only return LINUX_FD_CLOEXEC (1)

			// F_SETFD - Set the file descriptor flags
			//case LINUX_F_SETFD:
				// arg must be LINUX_FD_CLOEXEC (1)

			//
			// FILE STATUS FLAGS
			//

			// F_GETFL
			// F_SETFL

			//
			// ADVISORY RECORD LOCKING
			//

			// F_SETLK
			// F_SETLKW
			// F_GETLK

			//
			// OPEN FILE DESCRIPTOR LOCKS
			//

			// F_OFD_SETLK
			// F_OFD_SETLKW
			// F_OFD_GETLK

			//
			// MANDATORY LOCKING
			// (Do not support?)

			//
			// MANAGING SIGNALS
			//

			// F_GETOWN
			// F_SETOWN
			// F_GETOWN_EX
			// F_SETOWN_EX
			// F_GETSIG
			// F_SETSIG

			//
			// LEASES
			//

			// F_SETLEASE
			// F_GETLEASE

			//
			// FILE AND DIRECTORY CHANGE NOTIFICATION
			//

			// F_NOTIFY

			//
			// CHANGING THE CAPACITY OF A PIPE
			//

			// F_SETPIPE_SZ
			// F_GETPIPE_SZ

			default:
				_RPTF1(_CRT_ASSERT, "sys_fcntl: Unknown command %d", cmd);
				throw LinuxException(LINUX_EINVAL);
		}
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return -LINUX_EINVAL;
}

// sys32_fcntl64
//
sys32_long_t sys32_fcntl64(sys32_context_t context, sys32_int_t fd, sys32_int_t cmd, sys32_addr_t arg)
{
	return static_cast<sys32_long_t>(sys_fcntl(reinterpret_cast<SystemCall::Context*>(context), fd, cmd, reinterpret_cast<void*>(arg)));
}

#ifdef _M_X64
// sys64_fcntl
//
sys64_long_t sys64_fcntl(sys64_context_t context, sys64_int_t fd, sys64_int_t cmd, sys64_addr_t arg)
{
	return sys_fcntl(reinterpret_cast<SystemCall::Context*>(context), fd, cmd, reinterpret_cast<void*>(arg));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
