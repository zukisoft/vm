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

#ifndef __NATIVETHREAD_H_
#define __NATIVETHREAD_H_
#pragma once

#include <memory>
#include "NtApi.h"
#include "ProcessClass.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Class NativeThread
//
// Manages a native operating system thread in the host process instance

class NativeThread
{
public:

	// Destructor
	//
	~NativeThread();

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new NativeThread instance from an entry point and stack pointer
	static std::unique_ptr<NativeThread> Create(HANDLE process, void* entrypoint, void* stackpointer, size_t tlslength)
		{ return Create(process, entrypoint, stackpointer, nullptr, tlslength); }

	// Create (static)
	//
	// Creates a new NativeThread instance from an entry point, stack pointer and existing thread-local storage data
	static std::unique_ptr<NativeThread> Create(HANDLE process, void* entrypoint, void* stackpointer, void* tlsdata, size_t tlslength);

	// Create (static)
	//
	// Creates a new NativeThread instance using an existing task state
	static std::unique_ptr<NativeThread> Create(HANDLE process, void* taskstate, size_t taskstatelength, size_t tlslength)
		{ return Create(process, taskstate, taskstatelength, nullptr, tlslength); }
	
	// Create (static)
	//
	// Creates a new NativeThread instance using an existing task state and thread-local storage data
	static std::unique_ptr<NativeThread> Create(HANDLE process, void* taskstate, size_t taskstatelength, void* tlsdata, size_t tlslength);

	// Resume
	//
	// Resumes the thread from a suspended state
	void Resume(void);

	// Suspend
	//
	// Suspends the thread
	void Suspend(void);

	// Terminate
	//
	// Terminates the thread
	void Terminate(HRESULT exitcode);

	//-------------------------------------------------------------------------
	// Properties

	// ThreadHandle
	//
	// Gets the host main thread handle
	__declspec(property(get=getThreadHandle)) HANDLE ThreadHandle;
	HANDLE getThreadHandle(void) const { return m_thread; }

	// ThreadId
	//
	// Gets the host main thread identifier
	__declspec(property(get=getThreadId)) DWORD ThreadId;
	DWORD getThreadId(void) const { return m_threadid; }

	// TlsBase
	//
	// Gets the base address of the thread-local storage area
	const void* getTlsBase(void) const { return m_tlsbase; }

	// TlsLength
	//
	// Gets the length of the thread-local storage area
	size_t getTlsLength(void) const { return m_tlslength; }

private:

	NativeThread(const NativeThread&)=delete;
	NativeThread& operator=(const NativeThread&)=delete;

	// Instance Constructor
	//
	NativeThread(HANDLE thread, DWORD threadid, void* tlsbase, size_t tlslength) : m_thread(thread), m_threadid(threadid), m_tlsbase(tlsbase), m_tlslength(tlslength) {}
	friend std::unique_ptr<NativeThread> std::make_unique<NativeThread, HANDLE&, DWORD&, void*&, size_t&>(HANDLE&, DWORD&, void*&, size_t&);

	//-------------------------------------------------------------------------
	// Member Variables

	const HANDLE				m_thread;			// Native thread handle
	DWORD						m_threadid;			// Native thread id
	void*						m_tlsbase;			// TLS region base address
	size_t						m_tlslength;		// Length of the TLS region
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVETHREAD_H_
