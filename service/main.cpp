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
#include "resource.h"
#include "VmService.h"

#pragma warning(push, 4)			

//---------------------------------------------------------------------------
// _tWinMain
//
// Main application entry point
//
// Arguments :
//
//	instance		- Application instance handle
//	previnstance	- Unused in Win32, always set to NULL
//	cmdline			- Pointer to application command line string
//	show			- Application initial display flags

int APIENTRY _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{

#ifdef _DEBUG

	int nDbgFlags = _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG);	// Get current flags
	nDbgFlags |= _CRTDBG_LEAK_CHECK_DF;						// Enable leak-check
	_CrtSetDbgFlag(nDbgFlags);								// Set the new flags

#endif	// _DEBUG

	////////
	RPC_STATUS rpcresult = RpcServerUseAllProtseqsIf(RPC_C_PROTSEQ_MAX_REQS_DEFAULT, SystemCalls_v1_0_s_ifspec, nullptr);
	if(rpcresult != RPC_S_OK) {
	}
	///////

	// Use manual dispatching for now; haven't implemented a new ServiceModule<> class yet
	ServiceTable services = { ServiceTableEntry<VmService>(L"Instance Service") };
	services.Dispatch();

	return 0;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
