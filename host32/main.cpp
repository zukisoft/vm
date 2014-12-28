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
#include "SystemInformation.h"
#include <process.h>

#pragma warning(push, 4)
#pragma warning(disable:4731)	// frame pointer modified by inline assembly code

// g_rpccontext
//
// Global RPC context handle to the system calls server
sys32_context_t g_rpccontext;

// EmulationExceptionHandler (emulator.cpp)
//
// Vectored Exception handler used to provide emulation
LONG CALLBACK EmulationExceptionHandler(PEXCEPTION_POINTERS exception);

// t_gs (emulator.cpp)
//
// Emulated GS register value
extern __declspec(thread) uint32_t t_gs;

// t_ldt (emulator.cpp)
//
// Thread-local LDT
extern __declspec(thread) sys32_ldt_t t_ldt;

//-----------------------------------------------------------------------------
// ThreadMain
//
// Entry point for a hosted thread
//
// Arguments:
//
//	arg			- Argument passed to _beginthreadex (sys32_task_state_t)

unsigned __stdcall ThreadMain(void* arg)
{
	// Cast out the task state structure passed into the thread entry point
	sys32_task_state_t* taskstate = reinterpret_cast<sys32_task_state_t*>(arg);

	// Initialize the LDT as a copy of the provided LDT
	memcpy(&t_ldt, taskstate->ldt, sizeof(sys32_ldt_t));

	// Set up the emulated GS register for this thread
	t_gs = taskstate->gs;
	
	// Use the frame pointer to access the startup information fields;
	// this function will never return so it can be trashed
	__asm mov ebp, arg;

	// Set the general-purpose registers
	__asm mov eax, [ebp]sys32_task_state_t.eax;
	__asm mov ebx, [ebp]sys32_task_state_t.ebx;
	__asm mov ecx, [ebp]sys32_task_state_t.ecx;
	__asm mov edx, [ebp]sys32_task_state_t.edx;
	__asm mov edi, [ebp]sys32_task_state_t.edi;
	__asm mov esi, [ebp]sys32_task_state_t.esi;

	// Set the stack pointer and push the instruction pointer
	__asm mov esp, [ebp]sys32_task_state_t.esp;
	__asm push [ebp]sys32_task_state_t.eip;

	// Restore the frame pointer and jump via return
	__asm mov ebp, [ebp]sys32_task_state_t.ebp;
	__asm ret

	// Hosted thread never returns control back to here
	return static_cast<unsigned>(E_UNEXPECTED);
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
	zero_init<sys32_task_state_t>	taskstate;		// State information from the service
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
	hresult = sys32_acquire_context(binding, &taskstate, &g_rpccontext);
	if(FAILED(hresult)) return static_cast<int>(hresult);

	// Create a suspended thread that will execute the Linux binary
	HANDLE thread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, SystemInformation::AllocationGranularity, ThreadMain, &taskstate, CREATE_SUSPENDED, nullptr));
	if(thread == nullptr) { /* TODO: HANDLE THIS */ }

	// Install the emulator, which operates by intercepting low-level exceptions
	AddVectoredExceptionHandler(1, EmulationExceptionHandler);

	ResumeThread(thread);						// Launch the hosted process
	CloseHandle(thread);						// Finished with the thread handle

	// TODO: TEMPORARY - This thread will need to wait for signals and also shouldn't
	// die until every hosted thread has called exit() or some reasonable equivalent
	HANDLE delay = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	WaitForSingleObject(delay, INFINITE);

	// All hosted threads have terminated, release the RPC context
	return static_cast<int>(sys32_release_context(&g_rpccontext));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
