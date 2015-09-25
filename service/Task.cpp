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
#include "Task.h"

#pragma warning(push, 4)

// g_initonce (local)
//
// Global one-time initialization context
static INIT_ONCE g_initonce = INIT_ONCE_STATIC_INIT;

// InitOnceLoadModule (local)
//
// One-time initialization handler to load the KERNEL32.DLL module
static BOOL CALLBACK InitOnceLoadModule(PINIT_ONCE, PVOID, PVOID* context)
{
	*context = LoadLibrary(_T("kernel32.dll"));
	return TRUE;
}

// GetFunctionPointer (local)
//
// Retrieves a function pointer from the KERNEL32.DLL module
template<typename _funcptr> 
static _funcptr GetFunctionPointer(char const* name)
{
	HMODULE module;
	InitOnceExecuteOnce(&g_initonce, InitOnceLoadModule, nullptr, reinterpret_cast<PVOID*>(&module));
	return reinterpret_cast<_funcptr>(GetProcAddress(module, name));
}

#ifndef _M_X64
// 32-bit builds
static_assert(sizeof(CONTEXT) == sizeof(uapi::utask32), "uapi::utask32 structure is not equivalent to CONTEXT structure");
Task::GetThreadContext32Func const Task::GetThreadContext32 = GetFunctionPointer<Task::GetThreadContext32Func>("GetThreadContext");
Task::SetThreadContext32Func const Task::SetThreadContext32 = GetFunctionPointer<Task::SetThreadContext32Func>("SetThreadContext");
#else
// 64-bit builds
static_assert(sizeof(WOW64_CONTEXT) == sizeof(uapi::utask32), "uapi::utask32 structure is not equivalent to WOW64_CONTEXT structure");
static_assert(sizeof(CONTEXT) == sizeof(uapi::utask64), "uapi::utask64 structure is not equivalent to CONTEXT structure");
Task::GetThreadContext32Func const Task::GetThreadContext32 = GetFunctionPointer<Task::GetThreadContext32Func>("Wow64GetThreadContext");
Task::SetThreadContext32Func const Task::SetThreadContext32 = GetFunctionPointer<Task::SetThreadContext32Func>("Wow64SetThreadContext");
Task::GetThreadContext64Func const Task::GetThreadContext64 = GetFunctionPointer<Task::GetThreadContext64Func>("GetThreadContext");
Task::SetThreadContext64Func const Task::SetThreadContext64 = GetFunctionPointer<Task::SetThreadContext64Func>("SetThreadContext");
#endif

//-----------------------------------------------------------------------------
// Task Constructor
//
// Arugments:
//
//	architecture		- Task architecture
//	task				- Architecture-specific task information

Task::Task(enum class Architecture architecture, task_t&& task) : m_architecture(architecture), m_task(std::move(task))
{
}

//-----------------------------------------------------------------------------
// Task::getArchitecture
//
// Gets the architecture code for the contained task state

enum class Architecture Task::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// Task::Create<Architecture::x86> (static, private)
//
// Creates a new 32-bit task state
//
// Arguments:
//
//	instructionpointer	- Initial instruction pointer value
//	stackpointer		- Initial stack pointer value

template<>
std::unique_ptr<Task> Task::Create<Architecture::x86>(uintptr_t instructionpointer, uintptr_t stackpointer)
{
	task_t					task;				// New task information

#ifdef _M_X64
	// On x64, the pointers need to be validated as within the 32-bit address space
	if(instructionpointer > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, instructionpointer);
	if(stackpointer > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, stackpointer);
#endif

	memset(&task, 0, sizeof(task_t));
	task.x86.flags = UTASK32_FLAGS_INTEGER | UTASK32_FLAGS_CONTROL;

	// program counter
	task.x86.eip = static_cast<DWORD>(instructionpointer);

	// stack pointer
	task.x86.esp = static_cast<DWORD>(stackpointer);

	// processor flags
	task.x86.eflags = 0x200;			// EFLAGS_IF_MASK

	return std::make_unique<Task>(Architecture::x86, std::move(task));
}

//-----------------------------------------------------------------------------
// Task::Create<Architecture::x86_64> (static)
//
// Creates a new 64-bit task state
//
// Arguments:
//
//	instructionpointer	- Initial instruction pointer value
//	stackpointer		- Initial stack pointer value

#ifdef _M_X64
template<>
std::unique_ptr<Task> Task::Create<Architecture::x86_64>(uintptr_t instructionpointer, uintptr_t stackpointer)
{
	task_t					task;			// New task information

	memset(&task, 0, sizeof(task_t));
	task.x86_64.flags = UTASK64_FLAGS_FULL;

	// program counter
	task.x86_64.rip = static_cast<DWORD64>(instructionpointer);

	// stack pointer
	task.x86_64.rsp = static_cast<DWORD64>(stackpointer);

	// processor flags
	task.x86_64.eflags				= 0x200;	// EFLAGS_IF_MASK

	// sse
	task.x86_64.mxcsr				= 0x1F80;	// INITIAL_MXCSR

	// floating point
	task.x86_64.fltsave.controlword	= 0x027F;	// INITIAL_FPCSR
	task.x86_64.fltsave.mxcsr		= 0x1F80;	// INITIAL_MXCSR

	return std::make_unique<Task>(Architecture::x86_64, std::move(task));
}
#endif

//-----------------------------------------------------------------------------
// Task::Create (static)
//
// Creates a new architecture-specific task instance
//
// Arguments:
//
//	architecture		- Task architecture
//	instructionpointer	- Value to assign to the instruction pointer
//	stackpointer		- Value to assign to the stack pointer

std::unique_ptr<Task> Task::Create(enum class Architecture architecture, uintptr_t instructionpointer, uintptr_t stackpointer)
{
	if(architecture == Architecture::x86) return Create<Architecture::x86>(instructionpointer, stackpointer);
#ifdef _M_X64
	else if(architecture == Architecture::x86_64) return Create<Architecture::x86_64>(instructionpointer, stackpointer);
#endif

	// Unsupported architecture
	throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(architecture));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
