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
#include "SystemCall.h"

#include "SystemCallContext.h"
#include "Process.h"
#include "VirtualMachine.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys32_attach_process
//
// Acquires the process context for a 32-bit host
//
// Arguments:
//
//	rpchandle		- RPC binding handle
//	tid				- [in] native thread identifier of the process main thread
//	threadproc		- [in] address of the thread creation entry point
//	process			- [out] set to the process startup information
//	context			- [out] set to the newly allocated context handle

HRESULT sys32_attach_process(handle_t rpchandle, sys32_uint_t tid, sys32_addr_t threadproc, sys32_process_t* process, sys32_context_exclusive_t* context)
{
	uuid_t						objectid;			// RPC object identifier
	//Context*					handle = nullptr;	// System call context handle
	RPC_CALL_ATTRIBUTES			attributes;			// Client call attributes
	RPC_STATUS					rpcresult;			// Result from RPC function call

	// Acquire the object id for the interface connected to by the client
	rpcresult = RpcBindingInqObject(rpchandle, &objectid);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	// Acquire the attributes of the calling process
	memset(&attributes, 0, sizeof(RPC_CALL_ATTRIBUTES));
	attributes.Version = RPC_CALL_ATTRIBUTES_VERSION;
	attributes.Flags = RPC_QUERY_CLIENT_PID;

	rpcresult = RpcServerInqCallAttributes(rpchandle, &attributes);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	// TESTING
	//
	auto vm = VirtualMachine::Find(objectid);
	if(vm == nullptr) { /* TODO: THROW CUSTOM EXCEPTION */ }

	auto proc = Process::Attach(reinterpret_cast<DWORD>(attributes.ClientPID));
	if(proc == nullptr) { /* TODO: THROW CUSTOM EXCEPTION */ }
	//
	////////

	//try {

	//	// Use the RPC object id to locate the virtual machine instance
	//	auto vm = _VmOld::Find_VmOld(objectid);
	//	if(vm == nullptr) { /* TODO: THROW CUSTOM EXCEPTION */ }

	//	// Use the client's native process identifier to find the process
	//	auto proc = vm->FindNativeProcess(reinterpret_cast<DWORD>(attributes.ClientPID));
	//	if(proc == nullptr) { /* TODO: THROW CUSTOM EXCEPTION */ }
	//	
	//	// Use the process virtual PID to locate the thread, the first thread will match
	//	//auto thread = proc->Thread[proc->ProcessId];
	//	auto thread = proc->AttachThread(tid);
	//	if(thread == nullptr) { /* TODO: THROW CUSTOM EXCEPTION */ }

	//	// Set the provided address as the native thread entry point for the process
	//	proc->NativeThreadProc = reinterpret_cast<void*>(threadproc);

	//	// Acquire the necessary information for the process
	//	process->ldt = reinterpret_cast<sys32_addr_t>(proc->LocalDescriptorTable);
	//	thread->PopInitialTask(&process->task, sizeof(sys32_task_t));

	//	// Allocate the context handle by referencing the acquired objects
	//	handle = Context::Allocate(vm, proc, thread);
	//}

	//catch(const Exception& ex) { Context::Release(handle); return ex.HResult; }
	//catch(...) { Context::Release(handle); return E_FAIL; }

	//*context = reinterpret_cast<sys32_context_exclusive_t>(handle);
	//return S_OK;
	return E_FAIL;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
