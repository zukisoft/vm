//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#pragma warning(push, 4)
#pragma warning(disable:4731)	// frame pointer modified by inline assembly code

// g_ldt
//
// Pointer to the process-wide local descriptor table
void* g_ldt = nullptr;

// g_rpcbinding
//
// Global RPC binding handle to the system calls interface
RPC_BINDING_HANDLE g_rpcbinding = nullptr;

// t_exittask
//
// Thread-local task state information to restore host thread on exit
__declspec(thread) sys32_task_t t_exittask;

// t_gs
//
// Thread-local emulated GS segment register
__declspec(thread) uint16_t t_gs;

// t_rpccontext
//
// Thread-local RPC context handle for the system calls interface
__declspec(thread) sys32_context_t t_rpccontext;

// EmulationExceptionHandler (emulator.cpp)
//
// Vectored Exception handler used to provide emulation
LONG CALLBACK EmulationExceptionHandler(PEXCEPTION_POINTERS exception);

//-----------------------------------------------------------------------------
// ExecuteTask
//
// Executes a hosted task on the calling thread.  The hosted task must call the
// sys_exit() procedure, or an equivalent, in order for this to return control
// back to the original execution path
//
// Arguments:
//
//	task			- Task to be executed on this thread

DWORD ExecuteTask(sys32_task_t* task)
{
	sys32_task_t*	exittask = &t_exittask;		// Pointer to exit task state
	uint32_t		result;						// Result from the task (EAX)

	// Set the emulated GS segment register first
	t_gs = static_cast<uint16_t>(task->gs & 0xFFFF);

	// Use the EDI register as the pointer to the destination task state
	__asm push edi;
	__asm mov edi, exittask;

	// Save the current register state into the task structure
	__asm mov [edi]sys32_task_t.eax, eax;
	__asm mov [edi]sys32_task_t.ebx, ebx;
	__asm mov [edi]sys32_task_t.ecx, ecx;
	__asm mov [edi]sys32_task_t.edx, edx;
	// edi -> being used as the pointer; on the stack
	__asm mov [edi]sys32_task_t.esi, esi;
	__asm mov [edi]sys32_task_t.ebp, ebp;
	__asm mov [edi]sys32_task_t.esp, esp;

	// Set the instruction pointer such that it will return at the label below
	__asm mov eax, threadexit;
	__asm mov [edi]sys32_task_t.eip, eax;

	 // Use the ESI register as the pointer to the source task state
	__asm mov esi, task;

	// Set the current register state from the task structure
	__asm mov eax, [esi]sys32_task_t.eax;
	__asm mov ebx, [esi]sys32_task_t.ebx;
	__asm mov ecx, [esi]sys32_task_t.ecx;
	__asm mov edx, [esi]sys32_task_t.edx;
	__asm mov edi, [esi]sys32_task_t.edi;
	// esi -> being used as the pointer; set it last
	__asm mov ebp, [esi]sys32_task_t.ebp;
	__asm mov esp, [esi]sys32_task_t.esp;

	// Push the instruction pointer onto the new stack
	__asm push [esi]sys32_task_t.eip;

	// Set ESI and jump into the entry point via a RET instruction
	__asm mov esi, [esi]sys32_task_t.esi;
	__asm ret;

threadexit:

	// When the code gets back here, all registers except EDI will be set up,
	// EDI was pushed onto the stack before it was switched
	__asm pop edi;

	// The return code for the thread will have been set in the EAX register
	__asm mov result, eax;

	return result;
}

//-----------------------------------------------------------------------------
// ThreadMain
//
// Entry point for a thread created by the virtual machine service
//
// Arguments:
//
//	arg			- Argument passed to CreateThread

DWORD WINAPI ThreadMain(void*)
{
	zero_init<sys32_thread_t>	thread;			// Thread information from service
	//sys32_addr_t				cleartid = 0;	// Address to clear/signal on exit
	//DWORD						exitcode;		// Thread exit code

	// Attempt to acquire the task information and context handle from the server
	HRESULT hresult = sys32_attach_thread(g_rpcbinding, GetCurrentThreadId(), &thread, &t_rpccontext);
	if(FAILED(hresult)) return static_cast<DWORD>(hresult);

	//
	return sys32_exit(&t_rpccontext, ExecuteTask(&thread.task));

	//// Execute the task provided in the thread startup information
	//exitcode = ExecuteTask(&thread.task);

	//// Release the server context handle for this thread
	//sys32_release_thread(&t_rpccontext, static_cast<int>(exitcode), &cleartid);

	//// TODO: This is my best guess as to how I'm going to implement futex() - with
	//// the new WaitOnAddress/WakeByAddress functions, which are in-process only from
	//// what I can tell.  Therefore, the WakeBy() has to happen here in the host
	//if(cleartid) {

	//	// Zero the PID located at the provided address and signal the address
	//	*reinterpret_cast<uapi::pid_t*>(cleartid) = 0;
	//	WakeByAddressSingle(reinterpret_cast<void*>(cleartid));
	//}

	// Individual threads can return normally, do not call ExitThread()
	//return exitcode;
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
	zero_init<sys32_process_t>		process;		// Process information from the service
	RPC_STATUS						rpcresult;		// Result from RPC function call
	//sys32_addr_t					cleartid = 0;	// Address to clear/signal on exit
	//DWORD							exitcode;		// Exit code from the main thread
	HRESULT							hresult;		// Result from system call API function

	// EXPECTED ARGUMENTS:
	//
	// [0] - Executable path
	// [1] - RPC binding string
	if(__argc != 2) return static_cast<int>(ERROR_INVALID_PARAMETER);

	// The only argument passed into the host process is the RPC binding string necessary to connect to the server
	rpcresult = RpcBindingFromStringBinding(reinterpret_cast<rpc_tchar_t*>(__targv[1]), &g_rpcbinding);
	if(rpcresult != RPC_S_OK) return static_cast<int>(rpcresult);

	// Attempt to acquire the host process information and context from the server
	hresult = sys32_attach_process(g_rpcbinding, GetCurrentThreadId(), reinterpret_cast<sys32_addr_t>(ThreadMain), &process, &t_rpccontext);
	if(FAILED(hresult)) return static_cast<int>(hresult);

	// Set the pointer to the process-wide local descriptor table
	g_ldt = reinterpret_cast<void*>(process.ldt);

	// Install the emulator, which operates by intercepting low-level exceptions
	AddVectoredExceptionHandler(1, EmulationExceptionHandler);

	//
	ExitThread(sys32_exit(&t_rpccontext, ExecuteTask(&process.task)));

	//// Execute the task provided in the process startup information
	//exitcode = ExecuteTask(&process.task);

	//// Release the server context handle for this thread
	//sys32_release_thread(&t_rpccontext, static_cast<int>(exitcode), &cleartid);

	//// TODO: This is my best guess as to how I'm going to implement futex() - with
	//// the new WaitOnAddress/WakeByAddress functions, which are in-process only from
	//// what I can tell.  Therefore, the WakeBy() has to happen here in the host
	//if(cleartid) {

	//	// Zero the PID located at the provided address and signal the address
	//	*reinterpret_cast<uapi::pid_t*>(cleartid) = 0;
	//	WakeByAddressSingle(reinterpret_cast<void*>(cleartid));
	//}

	//// Call ExitThread rather than returning from WinMain, that would invoke ExitProcess()
	//// and kill any other threads that have been created inside this host process
	//ExitThread(exitcode);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
