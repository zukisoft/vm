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
#include "VmService.h"

#pragma warning(push, 4)			

//---------------------------------------------------------------------------
// VmService::OnStart (private)
//
// Invoked when the service is started
//
// Arguments :
//
//	argc		- Number of command line arguments
//	argv		- Array of command line argument strings

void VmService::OnStart(int, LPTSTR*)
{
	RPC_STATUS					rpcresult;			// Result from function call

	// Attept to register the remote system call RPC interface
	rpcresult = RpcServerRegisterIf(SystemCalls_v1_0_s_ifspec, nullptr, nullptr);
	if(rpcresult != RPC_S_OK) throw std::exception("RpcServerRegisterIf");

	// Start the RPC listener for this process
	rpcresult = RpcServerListen(1, RPC_C_LISTEN_MAX_CALLS_DEFAULT, 1);
	if(rpcresult != RPC_S_OK) {
		
		RpcServerUnregisterIf(SystemCalls_v1_0_s_ifspec, nullptr, 1);
		throw std::exception("RpcServerListen");
	}
}

//-----------------------------------------------------------------------------
// VmService::OnStop (private)
//
// Invoked when the service is stopped
//
// Arguments:
//
//	NONE

void VmService::OnStop(void)
{
	RpcMgmtStopServerListening(nullptr);
	RpcServerUnregisterIf(SystemCalls_v1_0_s_ifspec, nullptr, 1);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
