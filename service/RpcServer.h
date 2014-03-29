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

#ifndef __RPCSERVER_H_
#define __RPCSERVER_H_
#pragma once

#include "RpcException.h"			// Include RpcException declarations
#include "Win32Exception.h"			// Include Win32Exception declarations

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Class RpcServer

class RpcServer
{
public:

	// AddEndpoints (all protocols)
	//
	// Adds all mainline protocol sequences and automatically generates endpoints for them
	static void AddEndpoints(void)
		{ return AddEndpoints(RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 0, false); }

	static void AddEndpoints(uint32_t maxcalls)
		{ return AddEndpoints(maxcalls, 0, false); }

	static void AddEndpoints(uint32_t maxcalls, uint32_t flags)
		{ return AddEndpoints(maxcalls, flags, false); }

	static void AddEndpoints(uint32_t maxcalls, uint32_t flags, bool bindallnics);

	// AddEndpoints (single protocol)
	//
	// Adds a protocol sequence and automatically generates endpoints for it
	static void AddEndpoints(const tchar_t* protocol)
		{ return AddEndpoints(protocol, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 0, false); }

	static void AddEndpoints(const tchar_t* protocol, uint32_t maxcalls)
		{ return AddEndpoints(protocol, maxcalls, 0, false); }

	static void AddEndpoints(const tchar_t* protocol, uint32_t maxcalls, uint32_t flags)
		{ return AddEndpoints(protocol, maxcalls, flags, false); }

	static void AddEndpoints(const tchar_t* protocol, uint32_t maxcalls, uint32_t flags, bool bindallnics);

	static void AddEndpoints(const tchar_t* protocol, const tchar_t* endpoint)
		{ return AddEndpoints(protocol, endpoint, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 0, false); }

	static void AddEndpoints(const tchar_t* protocol, const tchar_t* endpoint, uint32_t maxcalls)
		{ return AddEndpoints(protocol, endpoint, maxcalls, 0, false); }

	static void AddEndpoints(const tchar_t* protocol, const tchar_t* endpoint, uint32_t maxcalls, uint32_t flags)
		{ return AddEndpoints(protocol, endpoint, maxcalls, flags, false); }

	static void AddEndpoints(const tchar_t* protocol, const tchar_t* endpoint, uint32_t maxcalls, uint32_t flags, bool bindallnics);

	// AddEndpoints (interface endpoints)
	//
	// Adds protocol sequence(s) and endpoints specified in an interface IDL declaration
	static void AddEndpoints(RPC_IF_HANDLE endpoints)
		{ return AddEndpoints(endpoints, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 0, false); }

	static void AddEndpoints(RPC_IF_HANDLE endpoints, uint32_t maxcalls)
		{ return AddEndpoints(endpoints, maxcalls, 0, false); }

	static void AddEndpoints(RPC_IF_HANDLE endpoints, uint32_t maxcalls, uint32_t flags)
		{ return AddEndpoints(endpoints, maxcalls, flags, false); }

	static void AddEndpoints(RPC_IF_HANDLE endpoints, uint32_t maxcalls, uint32_t flags, bool bindallnics);

	static void AddEndpoints(const tchar_t* protocol, RPC_IF_HANDLE endpoints)
		{ return AddEndpoints(protocol, endpoints, RPC_C_PROTSEQ_MAX_REQS_DEFAULT, 0, false); }

	static void AddEndpoints(const tchar_t* protocol, RPC_IF_HANDLE endpoints, uint32_t maxcalls)
		{ return AddEndpoints(protocol, endpoints, maxcalls, 0, false); }
	
	static void AddEndpoints(const tchar_t* protocol, RPC_IF_HANDLE endpoints, uint32_t maxcalls, uint32_t flags)
		{ return AddEndpoints(protocol, endpoints, maxcalls, flags, false); }

	static void AddEndpoints(const tchar_t* protocol, RPC_IF_HANDLE endpoints, uint32_t maxcalls, uint32_t flags, bool bindallnics);

private:

	RpcServer(const RpcServer& rhs);
	RpcServer& operator=(const RpcServer& rhs);

	//-------------------------------------------------------------------------
	// Member Variables
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCSERVER_H_
