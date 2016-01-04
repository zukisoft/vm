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

// sys_rt_sigprocmask.cpp
//
uapi::long_t sys_rt_sigprocmask(const Context* context, int how, const uapi::sigset_t* newmask, uapi::sigset_t* oldmask);

//-----------------------------------------------------------------------------
// sys_sigprocmask
//
// Sets the blocked signal mask for the calling thread
//
// Arguments:
//
//	context		- System call context object
//	how			- Flag indicating how newmask should be interpreted
//	newmask		- New signal mask to set for this thread
//	oldmask		- Receives the previously set thread signal mask

uapi::long_t sys_sigprocmask(const Context* context, int how, const uapi::old_sigset_t* newmask, uapi::old_sigset_t* oldmask)
{
	return -LINUX_ENOSYS;

	//// Watch out for sign-extension here, shouldn't happen as long as sigset_t and old_sigset_t are unsigned
	//static_assert(std::is_unsigned<uapi::sigset_t>::value && std::is_unsigned<uapi::old_sigset_t>::value, "signal mask data types must be unsigned to prevent sign-extension during conversion");

	//uapi::sigset_t convertnew, convertold;			// Compatible signal mask values
	//if(newmask) convertnew = *newmask;				// Convert the input mask value

	//// Invoke sys_rt_sigprocmask to execute the operation
	//uapi::long_t result = sys_rt_sigprocmask(context, how, (newmask) ? &convertnew : nullptr, &convertold);

	//// If the operation succeeded and the caller wants the old mask, trim it down and return it
	//if((result == 0) && (oldmask)) *oldmask = (convertold & 0xFFFFFFFF);

	//return result;
}

// sys32_sigprocmask
//
sys32_long_t sys32_sigprocmask(sys32_context_t context, int how, const sys32_old_sigset_t* newmask, sys32_old_sigset_t* oldmask)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_sigprocmask, context, how, reinterpret_cast<const uapi::old_sigset_t*>(newmask), reinterpret_cast<uapi::old_sigset_t*>(oldmask)));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
