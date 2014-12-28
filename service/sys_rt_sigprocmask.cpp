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
// sys_rt_sigprocmask
//
// Changes the list of currently blocked signals
//
// Arguments:
//
//	context		- SystemCall context object
//	how			- Option to add, remove or set signals
//	nset		- New signal mask to be applied
//	oset		- Optionally receives the old signal mask
//	sigsetsize	- Size of the caller's sigset_t structure

// move me into signal.h
namespace uapi {
typedef unsigned __int64 sigset_t;
};

__int3264 sys_rt_sigprocmask(const SystemCall::Context* context, int how, const uapi::sigset_t* nset, uapi::sigset_t* oset, size_t sigsetsize)
{
	_ASSERTE(context);

	// The size of sigset_t must currently match what is expected; the Linux kernel does this same check
	if(sigsetsize != sizeof(uapi::sigset_t)) return -LINUX_EINVAL;

	try { 
		
		SystemCall::Impersonation impersonation;

		// This should be pretty straightforward to implement as a property on the process object,
		// flesh out linux/signal.h and then add that

		(how);
		(nset);
		if(oset != nullptr) *oset = 0;		// <--- old mask goes here
		return 0;
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_rt_sigprocmask
//
sys32_long_t sys32_rt_sigprocmask(sys32_context_t context, sys32_int_t how, const sys32_sigset_t* nset, sys32_sigset_t* oset, sys32_size_t sigsetsize)
{
	return static_cast<sys32_long_t>(sys_rt_sigprocmask(reinterpret_cast<SystemCall::Context*>(context), how, nset, oset, sigsetsize));
}

#ifdef _M_X64
// sys64_rt_sigprocmask
//
sys64_long_t sys64_rt_sigprocmask(sys64_context_t context, sys64_int_t how, const sys64_sigset_t* nset, sys64_sigset_t* oset, sys64_size_t sigsetsize)
{
	return sys_rt_sigprocmask(reinterpret_cast<SystemCall::Context*>(context), how, nset, oset, sigsetsize);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
