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

#ifndef __SYSTEMCALL_H_
#define __SYSTEMCALL_H_
#pragma once

#include <syscalls32.h>
#ifdef _M_X64
#include <syscalls64.h>
#endif

// Forward Declarations
//
class Context;

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemCall
//
// System call invocation wrapper that provides the common initialization and
// termination code, exception handling, etc.
//
// Each wrapped system call must accept a const ContextHandle* as its first argument, 
// the remaining arguments are variadic and will be passed directly:
//
// uapi::long_t sys_sample(const ContextHandle* context, int argument1, int argument2);
//
// Invoking a system call from an RPC callback is accomplished by calling the
// variadic Invoke() method, passing the system call function and the RPC context handle 
// followed by the remaining arguments for the system call:
//
// sys32_long_t sys32_sample(sys32_context_t context, sys32_int_t arg1, sys32_int_t arg2
// {
//		return static_cast<sys32_long_t>(SystemCall::Invoke(sys_sample, context, arg1, arg2));
// }

class SystemCall
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Invoke<> (static)
	//
	// System call invocation wrapper
	template<typename _func, typename... _args>
	static uapi::long_t Invoke(const _func& func, void* context, _args&&... args)
	{
		uapi::long_t result = -1;				// Result from system call

		if(!context) return -LINUX_EFAULT;

		// Always impersonate the client prior to invoking the system call
		RPC_STATUS rpcresult = RpcImpersonateClient(nullptr);
		if(rpcresult != RPC_S_OK) return -LINUX_EPERM;

		// Invoke the system call inside of a generic exception handler
		try { result = func(reinterpret_cast<Context*>(context), std::forward<_args>(args)...); }
		catch(...) { result = TranslateException(std::current_exception()); }

		RpcRevertToSelf();					// Revert the impersonation
		return result;						// Return system call result
	}

	// TranslateException (static)
	//
	// Converts an exception into a return value for a system call
	static uapi::long_t TranslateException(std::exception_ptr ex);

private:

	SystemCall()=delete;
	~SystemCall()=delete;
	SystemCall(const SystemCall&)=delete;
	SystemCall& operator=(const SystemCall&)=delete;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMCALL_H_
