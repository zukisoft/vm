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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TaskState::getArchitecture
//
// Gets the architecture code for the contained task state

::Architecture TaskState::getArchitecture(void) const
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

std::unique_ptr<TaskState> TaskState::Capture(::Architecture architecture, HANDLE nativethread)
{
	context_t					context;		// Context acquired from the thread

	if(architecture == ::Architecture::x86) {

		// When capturing context, retrieve everything (CONTEXT_ALL)
		context.x86.ContextFlags = CONTEXT_ALL;

#ifndef _M_X64
		// Attempt to capture the context information for the specified thread
		if(!GetThreadContext(nativethread, &context.x86)) throw Win32Exception();
#else
		// Attempt to capture the context information for the specified thread
		if(!Wow64GetThreadContext(nativethread, &context.x86)) throw Win32Exception();
#endif
	}

#ifdef _M_X64
	else if(architecture == ::Architecture::x86_64) {

		// When capturing context, retrieve everything (CONTEXT_ALL)
		context.x86_64.ContextFlags = CONTEXT_ALL;
		if(!GetThreadContext(nativethread, &context.x86_64)) throw Win32Exception();
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
std::unique_ptr<TaskState> TaskState::Create<::Architecture::x86>(const void* entrypoint, const void* stackpointer)
{
	context_t				context;			// New context information

#ifdef _M_X64
	// On x64, the pointers need to be validated as within the 32-bit address space
	if(uintptr_t(entrypoint) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(entrypoint));
	if(uintptr_t(stackpointer) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(entrypoint));
#endif

	// When creating new context, only the integer and control registers are set
	memset(&context, 0, sizeof(context_t));
	context.x86.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

	// entrypoint -> EIP
	context.x86.Eip = reinterpret_cast<DWORD>(entrypoint);

	// stackpointer -> ESP/EBP
	context.x86.Esp = reinterpret_cast<DWORD>(stackpointer);
	context.x86.Ebp = reinterpret_cast<DWORD>(stackpointer);

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
std::unique_ptr<TaskState> TaskState::Create<::Architecture::x86_64>(const void* entrypoint, const void* stackpointer)
{
	context_t				context;			// New context information

	// When creating new context, only the integer and control registers are set
	memset(&context, 0, sizeof(context_t));
	context.x86_64.ContextFlags = CONTEXT_INTEGER | CONTEXT_CONTROL;

	// entrypoint -> RIP
	context.x86_64.Rip = reinterpret_cast<DWORD64>(entrypoint);

	// stackpointer -> RSP/RBP
	context.x86_64.Rsp = reinterpret_cast<DWORD64>(stackpointer);
	context.x86_64.Rbp = reinterpret_cast<DWORD64>(stackpointer);

	// Construct the TaskState from the generated context information
	return std::make_unique<TaskState>(Architecture::x86_64, std::move(context));
}
#endif

//-----------------------------------------------------------------------------
// TaskState::getData
//
// Gets a pointer to the underlying context information structure

const void* TaskState::getData(void) const
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

std::unique_ptr<TaskState> TaskState::Duplicate(const std::unique_ptr<TaskState>& existing)
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
std::unique_ptr<TaskState> TaskState::FromExisting<::Architecture::x86>(const void* existing, size_t length)
{
	context_t				context;			// Context information

	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(context32_t)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(context32_t));

	// Copy the context information and flags from the existing structure
	memcpy(&context.x86, existing, length);

	// Construct the TaskState from the copied context information
	return std::make_unique<TaskState>(::Architecture::x86, std::move(context));
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
std::unique_ptr<TaskState> TaskState::FromExisting<::Architecture::x86_64>(const void* existing, size_t length)
{
	context_t				context;			// Context information

	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(context64_t)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(context64_t));

	// Copy the context information and flags from the existing structure
	memcpy(&context.x86_64, existing, length);

	// Construct the TaskState from the copied context information
	return std::make_unique<TaskState>(::Architecture::x86_64, std::move(context));
}
#endif

//-----------------------------------------------------------------------------
// TaskState::getInstructionPointer
//
// Gets the contained xIP register value

const void* TaskState::getInstructionPointer(void) const
{
	// Architecture::x86 --> EIP
	if(m_architecture == Architecture::x86) return reinterpret_cast<void*>(m_context.x86.Eip);

#ifdef _M_X64
	// Architecture::x86_64 --> RIP
	else if(m_architecture == Architecture::x86_64) return reinterpret_cast<void*>(m_context.x86_64.Rip);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::putInstructionPointer
//
// Sets the contained xIP register value

void TaskState::putInstructionPointer(const void* value)
{
	// Architecture::x86 --> EIP
	if(m_architecture == Architecture::x86) {

		if(uintptr_t(value) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(value));
		m_context.x86.Eip = reinterpret_cast<DWORD>(value);
	}

#ifdef _M_X64
	// Architecture::x86_64 ---> RIP
	else if(m_architecture == Architecture::x86_64) m_context.x86_64.Rip = reinterpret_cast<DWORD64>(value);
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
	if(m_architecture == Architecture::x86) return sizeof(context32_t);

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit context
	else if(m_architecture == Architecture::x86_64) return sizeof(context64_t);
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

void TaskState::Restore(::Architecture architecture, HANDLE nativethread) const
{
	// Perform a sanity check to ensure that the target thread architecture
	// is the same as when this task state was captured or created
	if(architecture != m_architecture) 
		throw Exception(E_TASKSTATEWRONGCLASS, static_cast<int>(architecture), static_cast<int>(m_architecture));

	// Architecture::x86 --> 32-bit context
	if(m_architecture == Architecture::x86) {

#ifndef _M_X64
		if(!SetThreadContext(nativethread, &m_context.x86)) throw Win32Exception();
#else
		if(!Wow64SetThreadContext(nativethread, &m_context.x86)) throw Win32Exception();
#endif
	}

#ifdef _M_X64
	// Architecture::x86_64 --> 64-bit context
	else if(m_architecture == Architecture::x86_64) {

		// Restore the thread context using the same flags as when it was captured
		if(!SetThreadContext(nativethread, &m_context.x86_64)) throw Win32Exception();
	}
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
	if(m_architecture == Architecture::x86) return m_context.x86.Eax;

#ifdef _M_X64
	else if(m_architecture == Architecture::x86_64) return m_context.x86_64.Rax;
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
		m_context.x86.Eax = (value & 0xFFFFFFFF);
	}

#ifdef _M_X64
	// Architecture::x86_64 --> RAX
	else if(m_architecture == Architecture::x86_64) m_context.x86_64.Rax = value;
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::getStackPointer
//
// Gets the contained xSP register value

const void* TaskState::getStackPointer(void) const
{
	// Architecture::x86 --> ESP
	if(m_architecture == Architecture::x86) return reinterpret_cast<void*>(m_context.x86.Esp);

#ifdef _M_X64
	// Architecture::x86_64 --> RSP
	else if(m_architecture == Architecture::x86_64) return reinterpret_cast<void*>(m_context.x86_64.Rsp);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------
// TaskState::putStackPointer
//
// Sets the contained xSP register value

void TaskState::putStackPointer(const void* value)
{
	// Architecture::x86 --> ESP
	if(m_architecture == Architecture::x86) {

		if(uintptr_t(value) > UINT32_MAX) throw Exception(E_TASKSTATEOVERFLOW, uintptr_t(value));
		m_context.x86.Esp = reinterpret_cast<DWORD>(value);
	}

#ifdef _M_X64
	// Architecture::x86_64 ---> RSP
	else if(m_architecture == Architecture::x86_64) m_context.x86_64.Risp = reinterpret_cast<DWORD64>(value);
#endif

	// Unsupported architecture
	else throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(m_architecture));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
