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
#include "NativeThread.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// NativeThread Destructor

NativeThread::~NativeThread()
{
	CloseHandle(m_handle);
}

//-----------------------------------------------------------------------------
// NativeThread::Create<ProcessClass::x86> (static)
//
// Creates a new thread in the specified process
//
// Arguments:
//
//	process			- Handle to the native process
//	entrypoint		- Entry point for the thread
//	argument		- Argument to provide to the thread entry point

template<>
std::unique_ptr<NativeThread> NativeThread::Create<ProcessClass::x86>(HANDLE process, void* entrypoint, void* argument)
{
	DWORD					threadid;			// Newly created thread identifier

#ifdef _M_X64
	// Ensure that the entry point falls within the 32-bit address space
	if(uintptr_t(entrypoint) > UINT32_MAX) throw Exception(E_NATIVEPOINTER32);
#endif

	// Create the thread in the target process with the smallest allowable stack
	HANDLE handle = CreateRemoteThread(process, nullptr, SystemInformation::AllocationGranularity, reinterpret_cast<LPTHREAD_START_ROUTINE>(entrypoint),
		argument, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, &threadid);
	if(handle == nullptr) throw Win32Exception();

	// Create the NativeThread instance around the handle and thread identifier
	return std::make_unique<NativeThread>(ProcessClass::x86, handle, threadid);
}

//-----------------------------------------------------------------------------
// NativeThread::Create<ProcessClass::x86_64> (static)
//
// Creates a new thread in the specified process
//
// Arguments:
//
//	process			- Handle to the native process
//	entrypoint		- Entry point for the thread
//	argument		- Argument to provide to the thread entry point

#ifdef _M_X64
template<>
std::unique_ptr<NativeThread> NativeThread::Create<ProcessClass::x86_64>(HANDLE process, void* entrypoint, void* argument)
{
	DWORD					threadid;			// Newly created thread identifier

	// Create the thread in the target process with the smallest allowable stack
	HANDLE handle = CreateRemoteThread(process, nullptr, SystemInformation::AllocationGranularity, reinterpret_cast<LPTHREAD_START_ROUTINE>(entrypoint),
		argument, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, &threadid);
	if(handle == nullptr) throw Win32Exception();

	// Create the NativeThread instance around the handle and thread identifier
	return std::make_unique<NativeThread>(ProcessClass::x86_64, handle, threadid);
}
#endif

//-----------------------------------------------------------------------------
// NativeThread::Resume
//
// Resumes the native operating system thread
//
// Arguments:
//
//	NONE

void NativeThread::Resume(void)
{
	if(ResumeThread(m_handle) == static_cast<DWORD>(-1)) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// NativeThread::Suspend
//
// Suspends the native operating system thread
//
// Arguments:
//
//	NONE

void NativeThread::Suspend(void)
{
#ifdef _M_X64
	if(m_class == ProcessClass::x86) {

		// 32-bit processes running under WOW64 have a special version of SuspendThread
		if(Wow64SuspendThread(m_handle) == static_cast<DWORD>(-1)) throw Win32Exception();
		return;
	}
#endif

	// Native architecture threads use the standard SuspendThread function
	if(SuspendThread(m_handle) == static_cast<DWORD>(-1)) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// NativeThread::SuspendTask
//
// Suspends the native operating system thread and return the task state
//
// Arguments:
//
//	NONE

std::unique_ptr<NativeTask> NativeThread::SuspendTask(void)
{
#ifdef _M_X64
	if(m_class == ProcessClass::x86) {

		WOW64_CONTEXT context = { CONTEXT_ALL };

		// 32-bit processes running under WOW64 have a special version of SuspendThread
		if(Wow64SuspendThread(m_handle) == static_cast<DWORD>(-1)) throw Win32Exception();

		// 32-bit processes running under WOW64 have a special version of GetThreadContext
		if(!Wow64GetThreadContext(m_handle, &context)) throw Win32Exception();
		return NativeTask::FromExisting<WOW64_CONTEXT>(&context);
	}
#endif

	CONTEXT context = { CONTEXT_ALL };

	// Native architecture threads use the standard SuspendThread function
	if(SuspendThread(m_handle) == static_cast<DWORD>(-1)) throw Win32Exception();

	// Native architecture threads use the standard GetThreadContext function
	if(!GetThreadContext(m_handle, &context)) throw Win32Exception();

	return NativeTask::FromExisting<CONTEXT>(&context);
}

//-----------------------------------------------------------------------------
// NativeThread::Terminate
//
// Terminates the native operating system thread
//
// Arguments:
//
//	exitcode	- Exit code to use for thread termination

void NativeThread::Terminate(HRESULT exitcode)
{
	if(!TerminateThread(m_handle, static_cast<DWORD>(exitcode))) throw Win32Exception();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
