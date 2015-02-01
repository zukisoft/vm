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

// CONTEXT32 / CONTEXT64
//
// Aliases for either CONTEXT or WOW64_CONTEXT, depending on build type
#ifndef _M_X64
using CONTEXT32 = CONTEXT;
#else
using CONTEXT32 = WOW64_CONTEXT;
using CONTEXT64 = CONTEXT;
#endif

//-----------------------------------------------------------------------------
// NativeThread Destructor

NativeThread::~NativeThread()
{
	CloseHandle(m_thread);
}

//-----------------------------------------------------------------------------
// NativeThread::::Create (static)
//
// Creates a new thread in the specified process
//
// Arguments:
//
//	process			- Handle to the native process
//	entrypoint		- Entry point for the thread
//	stackpointer	- Stack pointer for the thread
//	tlssize			- Length required for the thread local storage

std::unique_ptr<NativeThread> NativeThread::Create(HANDLE process, void* entrypoint, void* stackpointer, size_t tlssize)
{
	DWORD						threadid;			// Thread ID
	zero_init<CONTEXT32>		context;			// Thread context information
	MEMORY_BASIC_INFORMATION	meminfo;			// Virtual memory information

	// Adjust the thread local storage size to be a multiple of the system allocation granularity
	tlssize = align::up(tlssize, SystemInformation::AllocationGranularity);

	// Create the thread in the target process using the specified entry point and thread local storage reservation
	HANDLE thread = CreateRemoteThread(process, nullptr, tlssize, reinterpret_cast<LPTHREAD_START_ROUTINE>(entrypoint), nullptr,
		CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, &threadid);
	if(thread == nullptr) throw Win32Exception();

	try {

		// Acquire the context information for the newly created thread
		context.ContextFlags = CONTEXT_ALL;
		if(!GetThreadContext(thread, &context)) throw Win32Exception();

		// Get the allocation base address for the native thread stack, this will be repurposed for thread local storage
		if(VirtualQueryEx(process, reinterpret_cast<LPCVOID>(context.Esp), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)) == 0) throw Win32Exception();
		void* tlsbase = meminfo.AllocationBase;

		// Commit the native thread stack and reset all protection to PAGE_READWRITE
		if(VirtualAllocEx(process, tlsbase, tlssize, MEM_COMMIT, PAGE_READWRITE) == nullptr) throw Win32Exception();

		// Set up the thread context to jump into the entry point and use the provided stack pointer
		context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
		context.Eax = context.Ebx = context.Ecx = context.Edx = context.Edi = context.Esi = 0;
		context.Eip = reinterpret_cast<DWORD>(entrypoint);
		context.Esp = reinterpret_cast<DWORD>(stackpointer);
		context.Ebp = reinterpret_cast<DWORD>(stackpointer);
		if(!SetThreadContext(thread, &context)) throw Win32Exception();

		// Create and return the NativeThread instance
		return std::make_unique<NativeThread>(thread, threadid, tlsbase, tlssize);
	} 
	
	catch(...) { CloseHandle(thread); throw; }
}

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
	if(ResumeThread(m_thread) == static_cast<DWORD>(-1)) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Host::Suspend
//
// Suspends the native operating system thread
//
// Arguments:
//
//	NONE

void NativeThread::Suspend(void)
{
	if(SuspendThread(m_thread) == static_cast<DWORD>(-1)) throw Win32Exception();
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
	if(!TerminateThread(m_thread, static_cast<DWORD>(exitcode))) throw Win32Exception();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
