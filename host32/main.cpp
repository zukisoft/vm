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
#include "Exception.h"
#include "Win32Exception.h"

// g_rpccontext
//
// Global RPC context handle to the system calls server
sys32_context_exclusive_t g_rpccontext;

//-----------------------------------------------------------------------------
// WinMain
//
// Application entry point
//
// Arguments:
//
//	hInstance			- Application instance handle (base address)
//	hPrevInstance		- Unused in Win32
//	pszCommandLine		- Pointer to the application command line
//	nCmdShow			- Initial window show command

int APIENTRY _tWinMain(HINSTANCE, HINSTANCE, LPTSTR, int)
{
	RPC_BINDING_HANDLE			binding;				// RPC binding from command line
	RPC_STATUS					rpcresult;				// Result from RPC function call
	HRESULT						hresult;				// Result from system call API function

	// EXPECTED ARGUMENTS:
	//
	// [0] - Executable path
	// [1] - RPC binding string
	// [2] - Inherited event handle (serialized as an int32_t)
	if(__argc != 3) return static_cast<int>(ERROR_INVALID_PARAMETER);

	// The only argument passed into the host process is the RPC binding string necessary to connect to the server
	rpcresult = RpcBindingFromStringBinding(reinterpret_cast<rpc_tchar_t*>(__targv[1]), &binding);
	if(rpcresult != RPC_S_OK) return static_cast<int>(rpcresult);

	// Attempt to acquire the host runtime context handle from the server
	hresult = sys32_acquire_context(binding, &g_rpccontext);
	if(FAILED(hresult)) return static_cast<int>(hresult);

	// The server will wait for an inherited event object to be signaled to ensure that this
	// process was able to acquire the context handle; set that event and close the handle
	HANDLE signal = reinterpret_cast<HANDLE>(_ttol(__targv[2]));
	// todo: this won't close the handle if SetEvent() fails
	if(!SetEvent(signal) || !CloseHandle(signal)) return static_cast<int>(GetLastError());

	//
	// TODO: just release the context and exit, more interesting things go here later
	//

	return static_cast<int>(sys32_release_context(&g_rpccontext));
}

//-----------------------------------------------------------------------------
