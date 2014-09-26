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

#ifndef __HOST_H_
#define __HOST_H_
#pragma once

#include <functional>
#include <memory>
#include <winternl.h>
#include "Exception.h"
#include "HeapBuffer.h"
#include "StructuredException.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Class Host
//
// Manages the Windows-based host process that is manipulated to run ELF binaries

class Host
{
public:

	// Destructor
	//
	~Host();

	// Create (static)
	//
	// Creates a new Host instance, optionally with an array of inheritable handles
	static std::unique_ptr<Host> Create(const tchar_t* path, const tchar_t* arguments) { return Create(path, arguments, nullptr, 0); }
	static std::unique_ptr<Host> Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles);

	// Resume
	//
	// Resumes the process from a suspended state
	void Resume(void);

	// Suspend
	//
	// Suspends the process
	void Suspend(void);

	// Terminate
	//
	// Terminates the process
	void Terminate(HRESULT exitcode);

	//-------------------------------------------------------------------------
	// Properties

	// ProcessHandle
	//
	// Gets the host process handle
	__declspec(property(get=getProcessHandle)) HANDLE ProcessHandle;
	HANDLE getProcessHandle(void) const { return m_procinfo.hProcess; }

	// ProcessId
	//
	// Gets the host process identifier
	__declspec(property(get=getProcessId)) DWORD ProcessId;
	DWORD getProcessId(void) const { return m_procinfo.dwProcessId; }

	// ThreadHandle
	//
	// Gets the host main thread handle
	__declspec(property(get=getThreadHandle)) HANDLE ThreadHandle;
	HANDLE getThreadHandle(void) const { return m_procinfo.hThread; }

	// ThreadId
	//
	// Gets the host main thread identifier
	__declspec(property(get=getThreadId)) DWORD ThreadId;
	DWORD getThreadId(void) const { return m_procinfo.dwThreadId; }

private:

	Host(const Host&)=delete;
	Host& operator=(const Host&)=delete;

	// Instance Constructor
	//
	Host(const PROCESS_INFORMATION& procinfo) : m_procinfo(procinfo) {}
	friend std::unique_ptr<Host> std::make_unique<Host, PROCESS_INFORMATION&>(PROCESS_INFORMATION&);

	//-------------------------------------------------------------------------
	// Member Variables

	PROCESS_INFORMATION								m_procinfo;
	static std::function<NTSTATUS(HANDLE handle)>	s_resumefunc;
	static std::function<NTSTATUS(HANDLE handle)>	s_suspendfunc;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __HOST_H_
