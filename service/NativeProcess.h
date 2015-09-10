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

#ifndef __NATIVEPROCESS_H_
#define __NATIVEPROCESS_H_
#pragma once

#include <memory>
#include "Architecture.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Class NativeProcess
//
// Creates a native operating system process instance

class NativeProcess
{
public:

	// Destructor
	//
	~NativeProcess();

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new NativeProcess instance
	static std::unique_ptr<NativeProcess> Create(const tchar_t* path, const tchar_t* arguments);
	static std::unique_ptr<NativeProcess> Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles);

	// Terminate
	//
	// Terminates the native process
	void Terminate(uint16_t exitcode) const;
	void Terminate(uint16_t exitcode, bool wait) const;
	
	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the (actual) architecture of the native process
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

	// ExitCode
	//
	// Gets the exit code from the native process
	__declspec(property(get=getExitCode)) DWORD ExitCode;
	DWORD getExitCode(void) const;

	// ProcessHandle
	//
	// Gets the host process handle
	__declspec(property(get=getProcessHandle)) HANDLE ProcessHandle;
	HANDLE getProcessHandle(void) const;

	// ProcessId
	//
	// Gets the host process identifier
	__declspec(property(get=getProcessId)) DWORD ProcessId;
	DWORD getProcessId(void) const;

	// ThreadHandle
	//
	// Gets the host main thread handle
	__declspec(property(get=getThreadHandle)) HANDLE ThreadHandle;
	HANDLE getThreadHandle(void) const;

	// ThreadId
	//
	// Gets the host main thread identifier
	__declspec(property(get=getThreadId)) DWORD ThreadId;
	DWORD getThreadId(void) const;

private:

	NativeProcess(const NativeProcess&)=delete;
	NativeProcess& operator=(const NativeProcess&)=delete;

	// Instance Constructor
	//
	NativeProcess(enum class Architecture architecture, PROCESS_INFORMATION& procinfo);
	friend std::unique_ptr<NativeProcess> std::make_unique<NativeProcess, enum class Architecture, PROCESS_INFORMATION&>(enum class Architecture&&, PROCESS_INFORMATION&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// GetProcessArchitecture (static)
	//
	// Determines the Architecture of a native process
	static enum class Architecture GetProcessArchitecture(HANDLE process);

	//-------------------------------------------------------------------------
	// Member Variables

	const enum class Architecture	m_architecture;		// Native process architecture
	const HANDLE					m_process;			// Native process handle
	DWORD							m_processid;		// Native process identifier
	const HANDLE					m_thread;			// Native thread handle
	DWORD							m_threadid;			// Native thread identifier
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVEPROCESS_H_
