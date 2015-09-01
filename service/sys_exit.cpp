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
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_exit
//
// Normal thread termination
//
// Arguments:
//
//	context		- System call context object
//	exitcode	- Exit code to report for the terminating thread

uapi::long_t sys_exit(const Context* context, int exitcode)
{
	return -LINUX_ENOSYS;

	//// Thread is terminating normally, inform the Process instance
	//context->Process->ExitThread(context->Thread->ThreadId, exitcode);
	//return 0;
}

// sys32_exit
sys32_long_t sys32_exit(sys32_context_exclusive_t* context_handle, sys32_int_t exitcode)
{
	// context_handle is [in, out, ref] for this system call
	Context* context = reinterpret_cast<Context*>(*context_handle);

	// Invoke the sys_exit system call and release the context handle if successful
	uapi::long_t result = static_cast<sys32_long_t>(SystemCall::Invoke(sys_exit, context, exitcode));
	if(result == 0) {

		Context::Release(context);				// Release the context object
		*context_handle = nullptr;				// Release the context handle
	}

	return result;
}

#ifdef _M_X64
// sys64_exit
//
void sys64_exit(sys64_exclusive_context_t* context_handle, sys64_int_t exitcode)
{
	// context_handle is [in, out, ref] for this system call
	Context* context = reinterpret_cast<Context*>(*context_handle);

	// Invoke the sys_exit system call and release the context handle if successful
	uapi::long_t result = SystemCall::Invoke(sys_exit, context, exitcode);
	if(result == 0) {

		Context::Release(context);				// Release the context object
		*context_handle = nullptr;				// Release the context handle
	}

	return result;
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
