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

#include <type_traits>
#include "ContextHandle.h"
#include "Win32Exception.h"

#include <syscalls32.h>
#ifdef _M_X64
#include <syscalls64.h>
#endif

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemCall
//
// Helper routines and classes used for implementation of system calls via RPC
// to allow interaction with the top-level Virtual Machine objects

class SystemCall
{
public:

	// Invoke
	//
	// Callable wrapper for any system call
	template<typename _func, typename... _args>
	static uapi::long_t Invoke(const _func& func, const ContextHandle* context, _args&&... args)
	{
		_ASSERTE(context);

		uapi::long_t result = -1;				// Result from system call

		// Always impersonate the client prior to invoking the system call
		RPC_STATUS rpcresult = RpcImpersonateClient(nullptr);
		if(rpcresult != RPC_S_OK) /* TODO: LOG THIS? */ return -LINUX_EPERM;

		try { result = func(context, std::forward<_args>(args)...); }
		catch(...) { result = TranslateException(std::current_exception()); }

		RpcRevertToSelf();						// Always revert impersonation
		return result;							// Return system call result
	}

	// TranslateException (static)
	//
	// Converts an exception into a return value for a system call
	static uapi::long_t TranslateException(std::exception_ptr ex)
	{
		try { std::rethrow_exception(ex); }
		catch(LinuxException& ex) {
			return -ex.Code;
		}
		// TODO: Win32Exception translation
		catch(Exception& ex) {

			int x = (int)ex.Code;
			return -x;
		}
		catch(...) { return -LINUX_ENOSYS; }

		return -1;			// <--- should be impossible to reach, shuts up the compiler
	}

private:

	~SystemCall()=delete;
	SystemCall(const SystemCall&)=delete;
	SystemCall& operator=(const SystemCall&)=delete;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMCALL_H_
