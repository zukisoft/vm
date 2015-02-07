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
#include "NativeTask.h"
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
	// Creates a new NativeThread instance within a native process
	template<ProcessClass _class>
	static std::unique_ptr<NativeThread> Create(HANDLE process, void* entrypoint) { return Create<_class>(process, entrypoint, nullptr); }

	// Create (static)
	//
	// Creates a new NativeThread instance within a native process
	template<ProcessClass _class>
	static std::unique_ptr<NativeThread> Create(HANDLE process, void* entrypoint, void* argument);

	// Resume
	//
	// Resumes the thread from a suspended state
	void Resume(void);

	// ResumeTask
	//
	// Resumes the thread from a suspended state with a new task
	void ResumeTask(const std::unique_ptr<NativeTask>& task);

	// Suspend
	//
	// Suspends the thread
	void Suspend(void);

	// SuspendTask
	//
	// Suspends the thread and returns the current task information
	std::unique_ptr<NativeTask> SuspendTask(void);

	// Terminate
	//
	// Terminates the thread
	void Terminate(HRESULT exitcode);

	//-------------------------------------------------------------------------
	// Properties

	// Handle
	//
	// Gets the host main thread handle
	__declspec(property(get=getThreadHandle)) HANDLE Handle;
	HANDLE getThreadHandle(void) const { return m_handle; }

	// ThreadId
	//
	// Gets the host main thread identifier
	__declspec(property(get=getThreadId)) DWORD ThreadId;
	DWORD getThreadId(void) const { return m_threadid; }

private:

	NativeThread(const NativeThread&)=delete;
	NativeThread& operator=(const NativeThread&)=delete;

	// Instance Constructor
	//
	NativeThread(ProcessClass _class, HANDLE handle, DWORD threadid) : m_class(_class), m_handle(handle), m_threadid(threadid) {}
	friend std::unique_ptr<NativeThread> std::make_unique<NativeThread, ProcessClass, HANDLE&, DWORD&>(ProcessClass&&, HANDLE&, DWORD&);

	//-------------------------------------------------------------------------
	// Member Variables

	const ProcessClass			m_class;			// Native thread class
	const HANDLE				m_handle;			// Native thread handle
	DWORD						m_threadid;			// Native thread id
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVETHREAD_H_
