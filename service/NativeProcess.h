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
#include "HeapBuffer.h"
#include "NativeHandle.h"
#include "SystemInformation.h"
#include "VirtualMachine.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// Class NativeProcess
//
// Creates a new native operating system host process

class NativeProcess
{
public:

	// Destructor
	//
	~NativeProcess()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Create<Architecture> (static)
	//
	// Creates a new NativeProcess instance for the specified architecture
	template<Architecture architecture>
	static std::unique_ptr<NativeProcess> Create(const std::shared_ptr<VirtualMachine>& vm);
	
	//-------------------------------------------------------------------------
	// Properties

	// Process
	//
	// Gets the host process handle
	__declspec(property(get=getProcess)) std::shared_ptr<NativeHandle> Process;
	std::shared_ptr<NativeHandle> getProcess(void) const;

	// ProcessId
	//
	// Gets the host process identifier
	__declspec(property(get=getProcessId)) DWORD ProcessId;
	DWORD getProcessId(void) const;

	// Thread
	//
	// Gets the host main thread handle
	__declspec(property(get=getThread)) std::shared_ptr<NativeHandle> Thread;
	std::shared_ptr<NativeHandle> getThread(void) const;

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
	NativeProcess(std::shared_ptr<NativeHandle>&& process, DWORD processid, std::shared_ptr<NativeHandle>&& thread, DWORD threadid);
	friend std::unique_ptr<NativeProcess> std::make_unique<NativeProcess, std::shared_ptr<NativeHandle>, DWORD&, std::shared_ptr<NativeHandle>, DWORD&>
		 (std::shared_ptr<NativeHandle>&&, DWORD&, std::shared_ptr<NativeHandle>&&, DWORD&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// CheckArchitecture (static)
	//
	// Checks the architecture of a constructed host process instance
	template<Architecture architecture>
	static void CheckArchitecture(const std::unique_ptr<NativeProcess>& instance);

	// Create (static)
	//
	// Creates a new NativeProcess instance
	static std::unique_ptr<NativeProcess> Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<NativeHandle>	m_process;		// Native process handle
	DWORD							m_processid;	// Native process identifier
	std::shared_ptr<NativeHandle>	m_thread;		// Native thread handle
	DWORD							m_threadid;		// Native thread identifier
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVEPROCESS_H_
