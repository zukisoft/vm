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

#include "stdafx.h"
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys32_acquire_context
//
// Acquires the system call context for a 32-bit client
//
// Arguments:
//
//	rpchandle		- RPC binding handle
//	startinfo		- [out] set to the startup information for the process
//	context			- [out] set to the newly allocated context handle

HRESULT sys32_acquire_context(handle_t rpchandle, sys32_startup_info* startinfo, sys32_context_exclusive_t* context)
{
	uuid_t						objectid;			// RPC object identifier
	SystemCall::Context*		handle = nullptr;	// System call context handle
	RPC_CALL_ATTRIBUTES			attributes;			// Client call attributes
	RPC_STATUS					rpcresult;			// Result from RPC function call

	// Acquire the object id for the interface connected to by the client, this will
	// indicate the VirtualMachine instance identifier
	rpcresult = RpcBindingInqObject(rpchandle, &objectid);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	// Acquire the attributes of the calling process for the Context object, this 
	// exposes the client process' PID for mapping to a Process object instance
	memset(&attributes, 0, sizeof(RPC_CALL_ATTRIBUTES));
	attributes.Version = RPC_CALL_ATTRIBUTES_VERSION;
	attributes.Flags = RPC_QUERY_CLIENT_PID;

	rpcresult = RpcServerInqCallAttributes(rpchandle, &attributes);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	try {

		SystemCall::Impersonation impersonation;		// Impersonate the client now

		// Allocate and initialize a Context object to be passed back as the RPC context
		handle = SystemCall::AllocContext(objectid, reinterpret_cast<uint32_t>(attributes.ClientPID));

		if(uintptr_t(handle->Process->EntryPoint) > UINT32_MAX) throw Win32Exception(ERROR_INVALID_ADDRESS);
		startinfo->entry_point = reinterpret_cast<sys32_addr_t>(handle->Process->EntryPoint);

		if(uintptr_t(handle->Process->StackPointer) > UINT32_MAX) throw Win32Exception(ERROR_INVALID_ADDRESS);
		startinfo->stack_pointer = reinterpret_cast<sys32_addr_t>(handle->Process->StackPointer);
	}

	catch(const Exception& ex) { SystemCall::FreeContext(handle); return ex.HResult; }
	catch(...) { SystemCall::FreeContext(handle); return E_FAIL; }

	*context = reinterpret_cast<sys32_context_exclusive_t>(handle);
	return S_OK;
}

#ifdef _M_X64
//-----------------------------------------------------------------------------
// sys64_acquire_context
//
// Acquires the system call context for a 64-bit client
//
// Arguments:
//
//	rpchandle		- RPC binding handle
//	startinfo		- [out] set to the startup information for the process
//	context			- [out] set to the newly allocated context handle

HRESULT sys64_acquire_context(handle_t rpchandle, sys64_startup_info* startinfo, sys64_context_exclusive_t* context)
{
	uuid_t						objectid;			// RPC object identifier
	SystemCall::Context*		handle = nullptr;	// System call context handle
	RPC_CALL_ATTRIBUTES			attributes;			// Client call attributes
	RPC_STATUS					rpcresult;			// Result from RPC function call

	// Acquire the object id for the interface connected to by the client, this will
	// indicate the VirtualMachine instance identifier
	rpcresult = RpcBindingInqObject(rpchandle, &objectid);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	// Acquire the attributes of the calling process for the Context object, this 
	// exposes the client process' PID for mapping to a Process object instance
	memset(&attributes, 0, sizeof(RPC_CALL_ATTRIBUTES));
	attributes.Version = RPC_CALL_ATTRIBUTES_VERSION;
	attributes.Flags = RPC_QUERY_CLIENT_PID;

	rpcresult = RpcServerInqCallAttributes(rpchandle, &attributes);
	if(rpcresult != RPC_S_OK) return HRESULT_FROM_WIN32(rpcresult);

	try {

		SystemCall::Impersonation impersonation;		// Impersonate the client now

		// Allocate and initialize a Context object to be passed back as the RPC context
		handle = SystemCall::AllocContext(objectid, reinterpret_cast<uint32_t>(attributes.ClientPID));

		startinfo->entry_point = reinterpret_cast<sys64_addr_t>(handle->Process->EntryPoint);
		startinfo->stack_image = reinterpret_cast<sys64_addr_t>(handle->Process->StackImage);
		startinfo->stack_image_length = static_cast<sys64_size_t>(handle->Process->StackImageLength);
	}

	catch(const Exception& ex) { SystemCall::FreeContext(handle); return ex.HResult; }
	catch(...) { SystemCall::FreeContext(handle); return E_FAIL; }

	*context = reinterpret_cast<sys32_context_exclusive_t>(handle);
	return S_OK;
}
#endif	// _M_X64

//---------------------------------------------------------------------------

#pragma warning(pop)
