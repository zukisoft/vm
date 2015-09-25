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
#include "TaskState.h"

#include "Exception.h"
#include "Win32Exception.h"

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
TaskState::GetThreadContext32Func const TaskState::GetThreadContext32 = GetFunctionPointer<TaskState::GetThreadContext32Func>("GetThreadContext");
TaskState::SetThreadContext32Func const TaskState::SetThreadContext32 = GetFunctionPointer<TaskState::SetThreadContext32Func>("SetThreadContext");
#else
// 64-bit builds
static_assert(sizeof(WOW64_CONTEXT) == sizeof(uapi::utask32), "uapi::utask32 structure is not equivalent to WOW64_CONTEXT structure");
static_assert(sizeof(CONTEXT) == sizeof(uapi::utask64), "uapi::utask64 structure is not equivalent to CONTEXT structure");
TaskState::GetThreadContext32Func const TaskState::GetThreadContext32 = GetFunctionPointer<TaskState::GetThreadContext32Func>("Wow64GetThreadContext");
TaskState::SetThreadContext32Func const TaskState::SetThreadContext32 = GetFunctionPointer<TaskState::SetThreadContext32Func>("Wow64SetThreadContext");
TaskState::GetThreadContext64Func const TaskState::GetThreadContext64 = GetFunctionPointer<TaskState::GetThreadContext64Func>("GetThreadContext");
TaskState::SetThreadContext64Func const TaskState::SetThreadContext64 = GetFunctionPointer<TaskState::SetThreadContext64Func>("SetThreadContext");
#endif

//-----------------------------------------------------------------------------
// TaskState::getArchitecture
//
// Gets the architecture code for the contained task state

enum class Architecture TaskState::getArchitecture(void) const
{
	return m_architecture;
}

//-----------------------------------------------------------------------------
// TaskState::Capture (static)
//
// Captures context from a suspended thread
//
// Arguments:
//
//	architecture	- Thread architecture
//	nativethread	- Native thread handle; should be suspended

std::unique_ptr<TaskState> TaskState::Capture(enum class Architecture architecture, HANDLE nativethread)
{
	context_t					context;		// Context acquired from the thread

	if(architecture == Architecture::x86) {

		context.x86.flags = UTASK32_FLAGS_FULL;
		if(!GetThreadContext32(nativethread, &context.x86)) throw Win32Exception();
	}

#ifdef _M_X64
	else if(architecture == Architecture::x86_64) {

		context.x86_64.flags = UTASK64_FLAGS_FULL;
		if(!GetThreadContext64(nativethread, &context.x86_64)) throw Win32Exception();
	}
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(architecture));

	// Construct the TaskState from the acquired context information
	return std::make_unique<TaskState>(Architecture::x86, std::move(context));
}

//-----------------------------------------------------------------------------
// TaskState::Create<Architecture::x86> (static)
//
// Creates a new 32-bit task state
//
// Arguments:
//
//	entrypoint		- Initial instruction pointer value
//	stackpointer	- Initial stack pointer value

template<>
std::unique_ptr<TaskState> TaskState::Create<Architecture::x86>(void const* entrypoint, void const* stackpointer)
{
	context_t				context;			// New context information

#ifdef _M_X64
	// On x64, the pointers need to be validated as within the 32-bit address space
	if(uintptr_t(entrypoint) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(entrypoint));
	if(uintptr_t(stackpointer) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(entrypoint));
#endif

	memset(&context, 0, sizeof(context_t));
	context.x86.flags = UTASK32_FLAGS_INTEGER | UTASK32_FLAGS_CONTROL;

	// program counter
	context.x86.eip = reinterpret_cast<DWORD>(entrypoint);

	// stack pointer
	context.x86.esp = reinterpret_cast<DWORD>(stackpointer);

	// processor flags
	context.x86.eflags = 0x200;			// EFLAGS_IF_MASK

	// Construct the TaskState from the generated context information
	return std::make_unique<TaskState>(Architecture::x86, std::move(context));
}

//-----------------------------------------------------------------------------
// TaskState::Create<Architecture::x86_64> (static)
//
// Creates a new 32-bit task state
//
// Arguments:
//
//	entrypoint		- Initial instruction pointer value
//	stackpointer	- Initial stack pointer value

#ifdef _M_X64
template<>
std::unique_ptr<TaskState> TaskState::Create<Architecture::x86_64>(void const* entrypoint, void const* stackpointer)
{
	context_t				context;			// New context information

	memset(&context, 0, sizeof(context_t));
	context.x86_64.flags = UTASK64_FLAGS_FULL;

	// program counter
	context.x86_64.rip = reinterpret_cast<DWORD64>(entrypoint);

	// stack pointer
	context.x86_64.rsp = reinterpret_cast<DWORD64>(stackpointer);

	// processor flags
	context.x86_64.eflags				= 0x200;	// EFLAGS_IF_MASK

	// sse
	context.x86_64.mxcsr				= 0x1F80;	// INITIAL_MXCSR

	// floating point
	context.x86_64.fltsave.controlword	= 0x027F;	// INITIAL_FPCSR
	context.x86_64.fltsave.mxcsr		= 0x1F80;	// INITIAL_MXCSR

	// Construct the TaskState from the generated context information
	return std::make_unique<TaskState>(Architecture::x86_64, std::move(context));
}
#endif

//-----------------------------------------------------------------------------
// TaskState::getData
//
// Gets a pointer to the underlying context information structure

void const* TaskState::getData(void) const
{
	// Architecture::x86 --> 32-bit context
	if(m_architecture == Architecture::x86) return &m_context.x86;

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit context
	else if(m_architecture == Architecture::x86_64) return &m_context.x86_64;
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::Duplicate (static)
//
// Duplicates an existing TaskState instance
//
// Arguments:
//
//	existing		- Existing task state to be duplicated

std::unique_ptr<TaskState> TaskState::Duplicate(std::unique_ptr<TaskState> const& existing)
{
	// Copy the existing context structure and construct a new TaskState instance
	context_t context = existing->m_context;
	return std::make_unique<TaskState>(existing->Architecture, std::move(context));
}

//-----------------------------------------------------------------------------
// TaskState::FromExisting<Architecture::x86> (static)
//
// Copies the task state information from an existing context structure
//
// Arguments:
//
//	existing		- Pointer to the existing task information
//	length			- Length of the existing task information

template<>
std::unique_ptr<TaskState> TaskState::FromExisting<Architecture::x86>(void const* existing, size_t length)
{
	context_t				context;			// Context information

	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(uapi::utask32)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(uapi::utask32));

	// Copy the context information and flags from the existing structure
	memcpy(&context.x86, existing, length);

	// Construct the TaskState from the copied context information
	return std::make_unique<TaskState>(Architecture::x86, std::move(context));
}

//-----------------------------------------------------------------------------
// TaskState::FromExisting<Architecture::x86_64> (static)
//
// Copies the task state information from an existing context structure
//
// Arguments:
//
//	existing		- Pointer to the existing task information
//	length			- Length of the existing task information

#ifdef _M_X64
template<>
std::unique_ptr<TaskState> TaskState::FromExisting<Architecture::x86_64>(void const* existing, size_t length)
{
	context_t				context;			// Context information

	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(uapi::utask64)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(uapi::utask64));

	// Copy the context information and flags from the existing structure
	memcpy(&context.x86_64, existing, length);

	// Construct the TaskState from the copied context information
	return std::make_unique<TaskState>(Architecture::x86_64, std::move(context));
}
#endif

//-----------------------------------------------------------------------------
// TaskState::getInstructionPointer
//
// Gets the contained xIP register value

void const* TaskState::getInstructionPointer(void) const
{
	// Architecture::x86 --> EIP
	if(m_architecture == Architecture::x86) return reinterpret_cast<void*>(m_context.x86.eip);

#ifdef _M_X64
	// Architecture::x86_64 --> RIP
	else if(m_architecture == Architecture::x86_64) return reinterpret_cast<void*>(m_context.x86_64.rip);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::putInstructionPointer
//
// Sets the contained xIP register value

void TaskState::putInstructionPointer(void const* value)
{
	// Architecture::x86 --> EIP
	if(m_architecture == Architecture::x86) {

		if(uintptr_t(value) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(value));
		m_context.x86.eip = reinterpret_cast<DWORD>(value);
	}

#ifdef _M_X64
	// Architecture::x86_64 ---> RIP
	else if(m_architecture == Architecture::x86_64) m_context.x86_64.rip = reinterpret_cast<DWORD64>(value);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::getLength
//
// Gets the length of the contained context information

size_t TaskState::getLength(void) const
{
	// Architecture::x86 --> 32-bit context
	if(m_architecture == Architecture::x86) return sizeof(uapi::utask32);

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit context
	else if(m_architecture == Architecture::x86_64) return sizeof(uapi::utask64);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//----------------------------------------------------------------------------
// TaskState::Restore<Architecture::x86>
//
// Restores the task state to a suspended 32-bit thread
//
// Arguments:
//
//	architecture	- Architecture of the target thread for verification
//	nativethread	- Native thread handle; should be suspended

void TaskState::Restore(enum class Architecture architecture, HANDLE nativethread) const
{
	// Perform a sanity check to ensure that the target thread architecture
	// is the same as when this task state was captured or created
	if(architecture != m_architecture) 
		throw Exception(E_TASKSTATEWRONGCLASS, static_cast<int>(architecture), static_cast<int>(m_architecture));

	// Architecture::x86 --> 32-bit context
	if(m_architecture == Architecture::x86)
		if(!SetThreadContext32(nativethread, &m_context.x86)) throw Win32Exception();

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit context
	else if(m_architecture == Architecture::x86_64)
		if(!SetThreadContext64(nativethread, &m_context.x86_64)) throw Win32Exception();
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::getReturnValue
//
// Gets the contained return value register value (EAX/RAX)

unsigned __int3264 TaskState::getReturnValue(void) const
{
	if(m_architecture == Architecture::x86) return m_context.x86.eax;

#ifdef _M_X64
	else if(m_architecture == Architecture::x86_64) return m_context.x86_64.rax;
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::putReturnValue
//
// Sets the contained return value register value (EAX/RAX)

void TaskState::putReturnValue(unsigned __int3264 value)
{
	// Architecture::x86 --> EAX
	if(m_architecture == Architecture::x86) {

		if(value > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, value);
		m_context.x86.eax = (value & 0xFFFFFFFF);
	}

#ifdef _M_X64
	// Architecture::x86_64 --> RAX
	else if(m_architecture == Architecture::x86_64) m_context.x86_64.rax = value;
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::getStackPointer
//
// Gets the contained xSP register value

void const* TaskState::getStackPointer(void) const
{
	// Architecture::x86 --> ESP
	if(m_architecture == Architecture::x86) return reinterpret_cast<void*>(m_context.x86.esp);

#ifdef _M_X64
	// Architecture::x86_64 --> RSP
	else if(m_architecture == Architecture::x86_64) return reinterpret_cast<void*>(m_context.x86_64.rsp);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::putStackPointer
//
// Sets the contained xSP register value

void TaskState::putStackPointer(void const* value)
{
	// Architecture::x86 --> ESP
	if(m_architecture == Architecture::x86) {

		if(uintptr_t(value) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(value));
		m_context.x86.esp = reinterpret_cast<DWORD>(value);
	}

#ifdef _M_X64
	// Architecture::x86_64 ---> RSP
	else if(m_architecture == Architecture::x86_64) m_context.x86_64.rsp = reinterpret_cast<DWORD64>(value);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
