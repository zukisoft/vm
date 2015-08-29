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

#include "Context.h"
#include "Thread.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_rt_sigprocmask
//
// Sets the blocked signal mask for the calling thread
//
// Arguments:
//
//	context		- System call context object
//	how			- Flag indicating how newmask should be interpreted
//	newmask		- New signal mask to set for this thread
//	oldmask		- Receives the previously set thread signal mask

uapi::long_t sys_rt_sigprocmask(const Context* context, int how, const uapi::sigset_t* newmask, uapi::sigset_t* oldmask)
{
	// Get the current signal mask for the thread in order to manipulate it
	uapi::sigset_t mask = context->Thread->SignalMask;

	// Apply the new mask bits to the existing signal mask if requested
	if(newmask) {

		switch(how) {

			// SIG_BLOCK: Add the new mask bits to the existing mask
			case LINUX_SIG_BLOCK: mask |= *newmask; break;

			// SIG_UNBLOCK: Remove the mask bits from the existing mask
			case LINUX_SIG_UNBLOCK: mask &= ~(*newmask); break;

			// SIG_SETMASK: Replace the existing mask with the new mask
			case LINUX_SIG_SETMASK: mask = *newmask; break;

			default: return -LINUX_EINVAL;
		}
	}

	// Set the updated signal mask for the thread and optionally return the old one
	context->Thread->SetSignalMask((newmask) ? &mask : nullptr, oldmask);
	return 0;
}

// sys32_rt_sigprocmask
//
sys32_long_t sys32_rt_sigprocmask(sys32_context_t context, int how, const sys32_sigset_t* newmask, sys32_sigset_t* oldmask)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_rt_sigprocmask, context, how, reinterpret_cast<const uapi::sigset_t*>(newmask), reinterpret_cast<uapi::sigset_t*>(oldmask)));
}

#ifdef _M_X64
// sys64_rt_sigprocmask
//
sys64_long_t sys64_rt_sigprocmask(sys64_context_t context, int how, const sys64_sigset_t* newmask, sys64_sigset_t* oldmask)
{
	return SystemCall::Invoke(sys_rt_sigprocmask, context, how, reinterpret_cast<const uapi::sigset_t*>(newmask), reinterpret_cast<uapi::sigset_t*>(oldmask));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
