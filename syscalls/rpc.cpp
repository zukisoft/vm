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

#include "stdafx.h"					// Include project pre-compiled headers

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Global Variables

// REMOTESYSTEMCALLS_TEMPLATE
//
// Remote system call service RPC binding handle template
static RPC_BINDING_HANDLE_TEMPLATE_V1 REMOTESYSTEMCALLS_TEMPLATE = {

	1,									// Version
	0,									// Flags
	RPC_PROTSEQ_LRPC,					// ProtocolSequence
	nullptr,							// NetworkAddress
	ENDPOINT_REMOTESYSTEMCALLS,			// StringEndpoint
	nullptr,							// Reserved
	GUID_NULL							// ObjectUuid
};

// t_rpchandle (TLS)
//
// Remote system call binding handle
__declspec(thread) static handle_t t_rpchandle = nullptr;

//-----------------------------------------------------------------------------
// midl_user_allocate
//
// Allocates RPC stub and library memory
//
// Arguments:
//
//	len			- Length of the required memory buffer

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	// Use the COM task memory allocator for RPC
	return CoTaskMemAlloc(len);
}

//-----------------------------------------------------------------------------
// midl_user_free
//
// Relases RPC stub and library memory
//
// Arguments:
//
//	ptr			- Pointer to buffer allocated by MIDL_user_allocate

void __RPC_USER midl_user_free(void __RPC_FAR* ptr)
{
	// Use the COM task memory allocator for RPC
	CoTaskMemFree(ptr);
}

//-----------------------------------------------------------------------------
// rpc_attach_thread
//
// Creates the RPC binding handle for a thread in respose to DLL_THREAD_ATTACH
//
// Arguments:
//
//	NONE

bool rpc_attach_thread(void)
{
	// Create the thread-local RPC binding handle from the global templates
	return (RpcBindingCreate(&REMOTESYSTEMCALLS_TEMPLATE, nullptr, nullptr, &t_rpchandle) == RPC_S_OK);
}

//---------------------------------------------------------------------------
// rpc_bind_thread
//
//
//
// Arguments:
//
//	NONE

__declspec(thread) static int bound = 0;
handle_t rpc_bind_thread(void)
{
	if(bound) return t_rpchandle;

	// TODO: Just call Bind() every time for now
	RPC_STATUS result = RpcBindingBind(nullptr, t_rpchandle, RemoteSystemCalls_v1_0_c_ifspec);
	bound = 1;
	return (result == RPC_S_OK) ? t_rpchandle : nullptr;
}

//-----------------------------------------------------------------------------
// rpc_detach_thread
//
// Frees the RPC binding handle for a thread in respose to DLL_THREAD_DETACH
//
// Arguments:
//
//	NONE

void rpc_detach_thread(void)
{
	RpcBindingFree(&t_rpchandle);			// Free the thread-local binding handle
}

//---------------------------------------------------------------------------

#pragma warning(pop)