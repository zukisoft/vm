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
#include "SystemCall.h"			// TODO: REMOVE ME?

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_release_thread
//
// Releases a thread context handle
//
// Arguments:
//
//	context		- System call context object to be released
//	exitcode	- Exit code to report for the thread object
//	cleartid	- Gets a pid_t address within the process to clear/signal

HRESULT sys_release_thread(Context* context, int exitcode, uintptr_t* cleartid)
{
	if(context == nullptr) return S_FALSE;

	// Get the optional address of a tid to clear and signal in the host
	// process from the thread object before its released
	*cleartid = uintptr_t(context->Thread->ClearThreadIdOnExit);

	// TODO -- this now signals the Schedulable object and 
	// sets the exit code; thread may be better served by keeping
	// a reference to its parent process and handling "RemoveThread" itself
	context->Thread->Exit(exitcode);

	// Remove the thread from the process instance
	context->Process->RemoveThread(context->Thread->ThreadId, exitcode);

	// Release the context object
	Context::Release(context);

	return S_OK;
}

// sys32_release_thread
//
HRESULT sys32_release_thread(sys32_context_exclusive_t* context, sys32_int_t exitcode, sys32_addr_t* cleartid)
{
	uintptr_t clearaddr = 0;				// Address to send back as cleartid

	// Release the thread context
	HRESULT result = sys_release_thread(reinterpret_cast<Context*>(*context), exitcode, &clearaddr);

	// Sanity check the address to send back for the 32-bit system call interface
	_ASSERTE(clearaddr <= UINT32_MAX);
	*cleartid = (clearaddr & 0xFFFFFFFF);

	*context = nullptr;						// Release raw context handle
	return result;							// Return operation result
}

#ifdef _M_X64
// sys64_release_context
//
HRESULT sys64_release_context(sys64_context_exclusive_t* context, sys64_int_t exitcode, sys64_addr_t* cleartid)
{
	uintptr_t clearaddr = 0;				// Address to send back as cleartid

	// Release the thread context
	HRESULT result = sys_release_thread(reinterpret_cast<Context*>(*context), exitcode, &clearaddr);

	// Pass the uintptr_t back to the caller as the address to clear/signal
	*cleartid = clearaddr;

	*context = nullptr;						// Release raw context handle
	return result;							// Return operation result
}
#endif	// _M_X64

//---------------------------------------------------------------------------

#pragma warning(pop)
