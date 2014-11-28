//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
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
// sys_rt_sigaction
//
// TODO

//__int3264 sys_rt_sigprocmask(const SystemCall::Context* context, int how, const uapi::sigset_t* nset, uapi::sigset_t* oset, size_t sigsetsize)
//{
//	_ASSERTE(context);
//
//	// The size of sigset_t must currently match what is expected; the Linux kernel does this same check
//	if(sigsetsize != sizeof(uapi::sigset_t)) return -LINUX_EINVAL;
//
//	try { 
//		
//		SystemCall::Impersonation impersonation;
//
//		// This should be pretty straightforward to implement as a property on the process object,
//		// flesh out linux/signal.h and then add that
//
//		(how);
//		(nset);
//		if(oset != nullptr) *oset = 0;		// <--- old mask goes here
//		return 0;
//	}
//
//	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
//}

// sys32_rt_sigaction
//
sys32_long_t sys32_rt_sigaction(sys32_context_t context, sys32_int_t sig, sys32_addr_t act, sys32_addr_t oact, sys32_size_t sigsetsize, sys32_addr_t restorer)
{
	(context); (sig); (act); (oact); (sigsetsize); (restorer);
	//return static_cast<sys32_long_t>(sys_rt_sigprocmask(reinterpret_cast<SystemCall::Context*>(context), how, nset, oset, sigsetsize));
	return 0;
}

//#ifdef _M_X64
//// sys64_rt_sigprocmask
////
//sys64_long_t sys64_rt_sigprocmask(sys64_context_t context, sys64_int_t how, const sys64_sigset_t* nset, sys64_sigset_t* oset, sys64_size_t sigsetsize)
//{
//	return sys_rt_sigprocmask(reinterpret_cast<SystemCall::Context*>(context), how, nset, oset, sigsetsize);
//}
//#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
