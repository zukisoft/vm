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

#include "stdafx.h"						// Include project pre-compiled headers
#include "RpcServer.h"					// Include RpcServer declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcServer::AddEndpoints (static)
//
// Adds all mainline protocols sequences with auto-generated endpoints
//
// Arguments:
//
//	maxcalls		- Backlog queue length (for ncacn_ip_tcp protocol)
//	flags			- Optional endpoint flags (for ncacn_ip_tcp/ncadg_ip_udp protocols)
//	bindallnics		- Flag to bind to all NICs when true, use registry when false

void RpcServer::AddEndpoints(uint32_t maxcalls, uint32_t flags, bool bindallnics)
{
	// Define the RPC_POLICY for the endpoints based on the provided arguments
	RPC_POLICY policy = { sizeof(RPC_POLICY), flags, (bindallnics) ? RPC_C_BIND_TO_ALL_NICS : 0 };

	// Attempt to register all of the endpoints defined for the interface
	RPC_STATUS result = RpcServerUseAllProtseqsEx(maxcalls, NULL, &policy);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------
// RpcServer::AddEndpoints (static)
//
// Adds a specific protocol sequence with auto-generated endpoints
//
// Arguments:
//
//	protocl			- Protocol sequence to add to the server
//	maxcalls		- Backlog queue length (for ncacn_ip_tcp protocol)
//	flags			- Optional endpoint flags (for ncacn_ip_tcp/ncadg_ip_udp protocols)
//	bindallnics		- Flag to bind to all NICs when true, use registry when false

void RpcServer::AddEndpoints(const tchar_t* protocol, uint32_t maxcalls, uint32_t flags, bool bindallnics)
{
	if(!protocol) throw Exception(E_POINTER);				// Protocol string cannot be NULL

	// RPC expects a non-const string pointer, allocate a copy of it on the stack
	size_t protlen = _tcslen(protocol) + 1;
	tchar_t* prot = reinterpret_cast<tchar_t*>(_alloca(protlen * sizeof(tchar_t)));
	_tcscpy_s(prot, protlen, protocol);

	// Define the RPC_POLICY for the endpoints based on the provided arguments
	RPC_POLICY policy = { sizeof(RPC_POLICY), flags, (bindallnics) ? RPC_C_BIND_TO_ALL_NICS : 0 };

	// Attempt to register all of the endpoints defined for the interface
	RPC_STATUS result = RpcServerUseProtseqEx(reinterpret_cast<RPC_TSTR>(prot), maxcalls, NULL, &policy);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------
// RpcServer::AddEndpoints (static)
//
// Adds a specific protocol sequence and endpoint
//
// Arguments:
//
//	protocol		- Specific protocol to use from the interface endpoints
//	endpoint		- Endpoint to register for the specified protocol sequence
//	maxcalls		- Backlog queue length (for ncacn_ip_tcp protocol)
//	flags			- Optional endpoint flags (for ncacn_ip_tcp/ncadg_ip_udp protocols)
//	bindallnics		- Flag to bind to all NICs when true, use registry when false

void RpcServer::AddEndpoints(const tchar_t* protocol, const tchar_t* endpoint, uint32_t maxcalls, uint32_t flags, bool bindallnics)
{
	if((!protocol) || (!endpoint)) throw Exception(E_POINTER);		// Cannot be NULL

	// RPC expects non-const string pointers, allocate copies of them on the stack
	size_t protlen = _tcslen(protocol) + 1;
	size_t eplen = _tcslen(endpoint) + 1;
	tchar_t* prot = reinterpret_cast<tchar_t*>(_alloca(protlen * sizeof(tchar_t)));
	tchar_t* ep = reinterpret_cast<tchar_t*>(_alloca(eplen * sizeof(tchar_t)));
	_tcscpy_s(prot, protlen, protocol);
	_tcscpy_s(ep, eplen, endpoint);

	// Define the RPC_POLICY for the endpoints based on the provided arguments
	RPC_POLICY policy = { sizeof(RPC_POLICY), flags, (bindallnics) ? RPC_C_BIND_TO_ALL_NICS : 0 };

	// Attempt to register all of the endpoints defined for the interface
	RPC_STATUS result = RpcServerUseProtseqEpEx(reinterpret_cast<RPC_TSTR>(prot), maxcalls, reinterpret_cast<RPC_TSTR>(ep), NULL, &policy);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------
// RpcServer::AddEndpoints (static)
//
// Adds all protocols sequences defined in an interface IDL declaration
//
// Arguments:
//
//	endpoints		- MIDL-defined server interface declaration
//	maxcalls		- Backlog queue length (for ncacn_ip_tcp protocol)
//	flags			- Optional endpoint flags (for ncacn_ip_tcp/ncadg_ip_udp protocols)
//	bindallnics		- Flag to bind to all NICs when true, use registry when false

void RpcServer::AddEndpoints(RPC_IF_HANDLE endpoints, uint32_t maxcalls, uint32_t flags, bool bindallnics)
{
	// Define the RPC_POLICY for the endpoints based on the provided arguments
	RPC_POLICY policy = { sizeof(RPC_POLICY), flags, (bindallnics) ? RPC_C_BIND_TO_ALL_NICS : 0 };

	// Attempt to register all of the endpoints defined for the interface
	RPC_STATUS result = RpcServerUseAllProtseqsIfEx(maxcalls, endpoints, NULL, &policy);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------
// RpcServer::AddEndpoints (static)
//
// Adds a specific protocol sequence defined in an interface IDL declaration
//
// Arguments:
//
//	protocol		- Specific protocol to use from the interface endpoints
//	endpoints		- MIDL-defined server interface declaration
//	maxcalls		- Backlog queue length (for ncacn_ip_tcp protocol)
//	flags			- Optional endpoint flags (for ncacn_ip_tcp/ncadg_ip_udp protocols)
//	bindallnics		- Flag to bind to all NICs when true, use registry when false

void RpcServer::AddEndpoints(const tchar_t* protocol, RPC_IF_HANDLE endpoints, uint32_t maxcalls, uint32_t flags, bool bindallnics)
{
	if(!protocol) throw Exception(E_POINTER);				// Protocol string cannot be NULL

	// RPC expects a non-const string pointer, allocate a copy of it on the stack
	size_t protlen = _tcslen(protocol) + 1;
	tchar_t* prot = reinterpret_cast<tchar_t*>(_alloca(protlen * sizeof(tchar_t)));
	_tcscpy_s(prot, protlen, protocol);

	// Define the RPC_POLICY for the endpoints based on the provided arguments
	RPC_POLICY policy = { sizeof(RPC_POLICY), flags, (bindallnics) ? RPC_C_BIND_TO_ALL_NICS : 0 };

	// Attempt to register all of the endpoints defined for the interface
	RPC_STATUS result = RpcServerUseProtseqIfEx(reinterpret_cast<RPC_TSTR>(prot), maxcalls, endpoints, NULL, &policy);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
