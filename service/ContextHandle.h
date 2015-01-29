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

#ifndef __CONTEXTHANDLE_H_
#define __CONTEXTHANDLE_H_
#pragma once

#include "Process.h"
#include "Thread.h"
#include "VirtualMachine.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// ContextHandle
//
// Object type used as the RPC context handle for a client process; maintains
// references to various virtual machine objects that are used to implement
// the system calls.  Instances of this class must be created and destroyed
// with the provided static Allocate() and Release() methods

class ContextHandle
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates a new ContextHandle instance
	static ContextHandle* Allocate(const std::shared_ptr<::VirtualMachine>& vm);
	static ContextHandle* Allocate(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process);
	static ContextHandle* Allocate(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process, const std::shared_ptr<::Thread>& thread);

	// Release
	//
	// Releases a ContextHandle instance
	static ContextHandle* Release(ContextHandle* context);

	//-------------------------------------------------------------------------
	// Properties

	// Process
	//
	// Gets the process object instance
	__declspec(property(get=getProcess)) std::shared_ptr<::Process> Process;
	std::shared_ptr<::Process> getProcess(void) const { return m_process; }

	// Thread
	//
	// Gets the thread object instance
	__declspec(property(get=getThread)) std::shared_ptr<::Thread> Thread;
	std::shared_ptr<::Thread> getThread(void) const { return m_thread; }

	// VirtualMachine
	//
	// Gets the contained VirtualMachine instance pointer
	__declspec(property(get=getVirtualMachine)) std::shared_ptr<::VirtualMachine> VirtualMachine;
	std::shared_ptr<::VirtualMachine> getVirtualMachine(void) const { return m_vm; }	

private:

	~ContextHandle()=default;
	ContextHandle(const ContextHandle&)=delete;
	ContextHandle& operator=(const ContextHandle&)=delete;

	// Instance Constructors
	//
	ContextHandle(const std::shared_ptr<::VirtualMachine>& vm);
	ContextHandle(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process);
	ContextHandle(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process, const std::shared_ptr<::Thread>& thread);

	//-------------------------------------------------------------------------
	// Member Variables

	std::shared_ptr<::VirtualMachine>	m_vm;			// VirtualMachine instance
	std::shared_ptr<::Process>			m_process;		// Process instance
	std::shared_ptr<::Thread>			m_thread;		// Thread instance
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __CONTEXTHANDLE_H_
