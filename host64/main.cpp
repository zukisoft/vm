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
#include "SystemInformation.h"
#include <process.h>

// g_rpccontext
//
// Global RPC context handle to the system calls server
sys64_context_t g_rpccontext;

//-----------------------------------------------------------------------------
// ElfMain
//
// Dummy thread to execute the loaded ELF binary code.  Created in a suspended
// state and then has it's context changed, this code should never actually run
//
// Arguments:
//
//	arg			- Argument passed to _beginthreadex

unsigned __stdcall ElfMain(void* arg)
{
	UNREFERENCED_PARAMETER(arg);

	// As stated above, this code should never actually execute
	_RPTF0(_CRT_ASSERT, "ElfMain thread is executing directly; context was not set");
	return 0;
}

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
	zero_init<sys64_task_state_t>	taskstate;		// Task state information from the service
	RPC_BINDING_HANDLE				binding;		// RPC binding from command line
	RPC_STATUS						rpcresult;		// Result from RPC function call
	HRESULT							hresult;		// Result from system call API function

	// EXPECTED ARGUMENTS:
	//
	// [0] - Executable path
	// [1] - RPC binding string
	if(__argc != 2) return static_cast<int>(ERROR_INVALID_PARAMETER);

	// The only argument passed into the host process is the RPC binding string necessary to connect to the server
	rpcresult = RpcBindingFromStringBinding(reinterpret_cast<rpc_tchar_t*>(__targv[1]), &binding);
	if(rpcresult != RPC_S_OK) return static_cast<int>(rpcresult);

	// Attempt to acquire the host runtime context handle from the server
	hresult = sys64_acquire_context(binding, &taskstate, &g_rpccontext);
	if(FAILED(hresult)) return static_cast<int>(hresult);

	// Create a suspended thread that will execute the Linux binary
	HANDLE thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, static_cast<unsigned int>(SystemInformation::AllocationGranularity), 
		ElfMain, nullptr, CREATE_SUSPENDED, nullptr));
	if(thread == nullptr) { /* TODO: HANDLE THIS */ }

	// TODO: WORDS
	zero_init<CONTEXT> context;
	context.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;
	
	// Acquire the current thread CONTEXT information for registers that aren't going to be changed
	if(!GetThreadContext(thread, &context)) { /* TODO: HANDLE THIS */ }

	// Change the general purpose and control registers to the values provided
	context.Rax = taskstate.rax;
	context.Rbx = taskstate.rbx;
	context.Rcx = taskstate.rcx;
	context.Rdx = taskstate.rdx;
	context.Rdi = taskstate.rdi;
	context.Rsi = taskstate.rsi;
	context.R8  = taskstate.r8;
	context.R9  = taskstate.r9;
	context.R10 = taskstate.r10;
	context.R11 = taskstate.r11;
	context.R12 = taskstate.r12;
	context.R13 = taskstate.r13;
	context.R14 = taskstate.r14;
	context.R15 = taskstate.r15;
	context.Rbp = taskstate.rbp;
	context.Rip = taskstate.rip;
	context.Rsp = taskstate.rsp;

	// Apply the updated CONTEXT information to the suspended thread
	if(!SetThreadContext(thread, &context)) { /* TODO: HANDLE THIS */ }

	ResumeThread(thread);						// Launch the hosted process
	CloseHandle(thread);						// Finished with the thread handle

	// TODO: TEMPORARY - This thread will need to wait for signals and also shouldn't
	// die until every hosted thread has called exit() or some reasonable equivalent
	HANDLE delay = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	WaitForSingleObject(delay, INFINITE);

	// All hosted threads have terminated, release the RPC context
	return static_cast<int>(sys64_release_context(&g_rpccontext));
}

//-----------------------------------------------------------------------------
