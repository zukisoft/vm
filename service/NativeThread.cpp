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
#define GetThreadContext32	GetThreadContext
#define SetThreadContext32	SetThreadContext
#else
using CONTEXT32 = WOW64_CONTEXT;
using CONTEXT64 = CONTEXT;
#define GetThreadContext32	Wow64GetThreadContext
#define GetThreadContext64	GetThreadContext
#define SetThreadContext32	Wow64SetThreadContext
#define SetThreadContext64	SetThreadContext
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
//	tlsdata			- Data to be copied into the thread-local storage area or null
//	tlslength		- Length required for the thread-local storage

std::unique_ptr<NativeThread> NativeThread::Create(HANDLE process, void* entrypoint, void* stackpointer, void* tlsdata, size_t tlslength)
{
	zero_init<CONTEXT32>		context;			// Thread context information

	// Apply the provided entry point and stack pointer to the dummy context
	context.Ebp = reinterpret_cast<DWORD>(stackpointer);
	context.Eip = reinterpret_cast<DWORD>(entrypoint);
	context.Esp = reinterpret_cast<DWORD>(stackpointer);

	// Invoke the creation function that accepts the task state structure
	return Create(process, &context, sizeof(CONTEXT32), tlsdata, tlslength);
}

//-----------------------------------------------------------------------------
// NativeThread::::Create<CONTEXT32> (static)
//
// Creates a new thread in the specified process
//
// Arguments:
//
//	process			- Handle to the native process
//	taskstate		- Pointer to an existing task state structure
//	taskstatelength	- Length of the task state information structure
//	tlsdata			- Data to be copied into the thread-local storage area or null
//	tlslength		- Length required for the thread-local storage

std::unique_ptr<NativeThread> NativeThread::Create(HANDLE process, void* taskstate, size_t taskstatelen, void* tlsdata, size_t tlslength)
{
	DWORD						threadid;			// Thread ID
	CONTEXT32					context;			// Thread context information
	MEMORY_BASIC_INFORMATION	meminfo;			// Virtual memory information

	// TODO: temporary - need to switch out with a template class for x86/x64
	if(taskstatelen != sizeof(CONTEXT32)) throw Exception(E_INVALIDARG);
	CONTEXT32* existing = reinterpret_cast<CONTEXT32*>(taskstate);

	// Create the thread in the target process using the specified entry point and thread-local storage reservation
	HANDLE thread = CreateRemoteThread(process, nullptr, align::up(tlslength, SystemInformation::AllocationGranularity), 
		nullptr, nullptr, CREATE_SUSPENDED | STACK_SIZE_PARAM_IS_A_RESERVATION, &threadid);
	if(thread == nullptr) throw Win32Exception();

	//
	// 64K stack
	//
	// 48K unused
	// 8K guard
	// 8K stack

	// is it useful to push to the stack or just allocate another section


	//
	// TODO: EAX NEEDS TO BE SET TO THE ENTRY POINT, CREATETHREAD() INVOKES RTLUSERTHREAD WHICH
	// NEEDS TO DO ESSENTIAL SET UP OTHERWISE THIS WON'T WORK
	// SHOULD BE OK TO MUCK WITH THE STACK POINTER, THOUGH
	//

	try {

		// Acquire the context information for the newly created thread
		context.ContextFlags = CONTEXT_ALL;
		if(!GetThreadContext32(thread, &context)) throw Win32Exception();

		// Get the allocation base address for the native thread stack, this will be repurposed for thread local storage
		if(VirtualQueryEx(process, reinterpret_cast<LPCVOID>(context.Esp), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)) == 0) throw Win32Exception();
		void* tlsbase = meminfo.AllocationBase;

		// Commit the native thread stack and reset the protection to PAGE_READWRITE
		if(VirtualAllocEx(process, tlsbase, tlslength, MEM_COMMIT, PAGE_READWRITE) == nullptr) throw Win32Exception();

		// If TLS initialization data has been provided, copy that into the repurposed memory region
		if(tlsdata) NtApi::NtWriteVirtualMemory(process, tlsbase, tlsdata, tlslength, nullptr);

		// Apply only the INTEGER and CONTROL flags from the provided task state structure
		context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
		context.Eax = existing->Eax;
		context.Ebx = existing->Ebx;
		context.Ecx = existing->Ecx;
		context.Edx = existing->Edx;
		context.Edi = existing->Edi;
		context.Esi = existing->Esi;
		context.Ebp = existing->Ebp;
		context.Eip = existing->Eip;
		context.Esp = existing->Esp;

		// Attempt to set the task state to the suspended thread object
		if(!SetThreadContext32(thread, &context)) throw Win32Exception();

		// Create and return the NativeThread instance
		return std::make_unique<NativeThread>(thread, threadid, tlsbase, tlslength);
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
