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

#ifndef __SYSTEMCALL_H_
#define __SYSTEMCALL_H_
#pragma once

#include "Process.h"
#include "VirtualMachine.h"
#include "Win32Exception.h"

#include <syscalls32.h>
#ifdef _M_X64
#include <syscalls64.h>
#endif

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// SystemCall
//
// Helper routines and classes used for implementation of system calls via RPC
// to allow interaction with the top-level Virtual Machine objects

class SystemCall
{
public:

	// Context
	//
	// Object type used as the RPC context handle for a client process; maintains
	// a reference to the target VirtualMachine as well as the Process instance
	// associated with the caller
	//
	// Instances of this class must be created/destroyed with the provided static
	// functions as midl_user_allocate/midl_user_free are required by RPC
	class Context
	{
	friend class SystemCall;
	public:

		// Process
		//
		// Gets the process object instance
		__declspec(property(get=getProcess)) std::shared_ptr<::Process> Process;
		std::shared_ptr<::Process> getProcess(void) const { return m_process; }

		// VirtualMachine
		//
		// Gets the contained VirtualMachine instance pointer
		__declspec(property(get=getVirtualMachine)) std::shared_ptr<::VirtualMachine> VirtualMachine;
		std::shared_ptr<::VirtualMachine> getVirtualMachine(void) const { return m_vm; }	
	
	private:

		~Context()=default;
		Context(const Context&)=delete;
		Context& operator=(const Context&)=delete;

		// Instance Constructor
		//
		Context(const std::shared_ptr<::VirtualMachine>& vm, const std::shared_ptr<::Process>& process) : m_vm(vm), m_process(process) {}

		// Member Variables
		//
		std::shared_ptr<::VirtualMachine>	m_vm;		// VirtualMachine instance
		std::shared_ptr<::Process>			m_process;	// Process instance
	};

	// AllocContext (static)
	//
	// Constructs a new Context for use by an RPC system call client
	static Context* AllocContext(const uuid_t instanceid, uint32_t hostpid)
	{
		auto vm = VirtualMachine::FindVirtualMachine(instanceid);
		auto process = vm->FindProcessByHostID(hostpid);

		// Allocate the backing storage for the Context with MIDL_user_allocate
		void* context = MIDL_user_allocate(sizeof(Context));
		if(!context) throw Exception(E_OUTOFMEMORY);

		// Use placement new to construct the Context on the allocated storage
		return new(context) Context(vm, process);
	}

	// FreeContext (static)
	//
	// Destroys and releases a Context instance
	static void FreeContext(Context* context)
	{
		if(!context) return;

		context->~Context();			// Invoke destructor
		MIDL_user_free(context);		// Release backing storage
	}

	// TranslateException (static)
	//
	// Converts an exception into a return value for a system call
	static __int3264 TranslateException(std::exception_ptr ex)
	{
		try { std::rethrow_exception(ex); }
		catch(LinuxException& ex) {
			return -ex.Code;
		}
		catch(Exception& ex) {

			int x = (int)ex.Code;
			return -x;
		}
		catch(...) { return -LINUX_ENOSYS; }

		return -1;			// <--- should be impossible to reach, shuts up the compiler
	}

	// Impersonation
	//
	// Class used to automate the calls to RpcImpersonateClient/RpcRevertToSelf, an
	// instance of this should be present in every server-side system call to ensure
	// that the virtual machine is accessed under the calling process' identity:
	class Impersonation
	{
	public:

		// Constructor
		//
		Impersonation()
		{
			RPC_STATUS result = RpcImpersonateClient(nullptr);
			if(result != RPC_S_OK) throw Win32Exception(result);
		}

		// Destructor
		//
		~Impersonation() { RpcRevertToSelf(); }

	private:

		Impersonation(const Impersonation&)=delete;
		Impersonation& operator=(const Impersonation&)=delete;
	};

private:

	~SystemCall()=delete;
	SystemCall(const SystemCall&)=delete;
	SystemCall& operator=(const SystemCall&)=delete;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMCALL_H_
