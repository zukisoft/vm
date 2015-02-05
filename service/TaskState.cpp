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

// Include the RPC headers here rather than the header file to prevent them
// from being implicitly included in anything that references this class
#include <syscalls32.h>
#ifdef _M_X64
#include <syscalls64.h>
#endif

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TaskState::CopyTo
//
// Copies the task state information from the internal buffer, the destination
// size must be an exact match to the internal buffer's size to ensure that the
// process class is accurate
//
// Arguments:
//
//	taskstate		- Output buffer to receive the task state information
//	length			- Length of the output buffer, in bytes

void TaskState::CopyTo(void* taskstate, size_t length) const
{
	if(taskstate == nullptr) throw Exception(E_POINTER);
	if(length != m_blob.Size) throw Exception(E_TASKSTATEINVALIDLENGTH, length, m_blob.Size);

	// The size matches, just copy the data from the contained buffer
	memcpy(taskstate, &m_blob, m_blob.Size);
}

//-----------------------------------------------------------------------------
// TaskState::Create<ProcessClass::x86> (static, private)
//
// Constructs a TaskState for a new process or thread
//
// Arguments:
//
//	_class			- Process class designation
//	entrypoint		- Address of the process/thread entry point
//	stackpointer	- Address of the process/thread stack pointer

template <>
std::unique_ptr<TaskState> TaskState::Create<ProcessClass::x86>(const void* entrypoint, const void* stackpointer)
{
	HeapBuffer<uint8_t> blob(sizeof(sys32_task_t));
	sys32_task_t* state = reinterpret_cast<sys32_task_t*>(&blob);

	// Integer Registers
	state->eax = 0;
	state->ebx = 0;
	state->ecx = 0;
	state->edx = 0;
	state->edi = 0;
	state->esi = 0;

	// Control Registers
	state->eip = reinterpret_cast<sys32_addr_t>(entrypoint);
	state->ebp = reinterpret_cast<sys32_addr_t>(stackpointer);
	state->esp = reinterpret_cast<sys32_addr_t>(stackpointer);

	// Segment Registers
	state->gs = 0;

	return std::make_unique<TaskState>(std::move(blob));
}

//-----------------------------------------------------------------------------
// TaskState::Create<ProcessClass::x86> (static, private)
//
// Constructs a TaskState from an existing task state blob
//
// Arguments:
//
//	existing		- Pointer to the existing task state blob
//	length			- Length of the existing task state blob

template <>
std::unique_ptr<TaskState> TaskState::Create<ProcessClass::x86>(const void* existing, size_t length)
{
	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(sys32_task_t)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(sys32_task_t));

	// Copy the existing task information into a new HeapBuffer<> instance
	HeapBuffer<uint8_t> blob(sizeof(sys32_task_t));
	memcpy(blob, existing, length);

	return std::make_unique<TaskState>(std::move(blob));
}

#ifdef _M_X64

//-----------------------------------------------------------------------------
// TaskState::Create<ProcessClass::x86_64> (static, private)
//
// Constructs a TaskState for a new process or thread
//
// Arguments:
//
//	_class			- Process class designation
//	entrypoint		- Address of the process/thread entry point
//	stackpointer	- Address of the process/thread stack pointer

template <>
std::unique_ptr<TaskState> TaskState::Create<ProcessClass::x86_64>(const void* entrypoint, const void* stackpointer)
{
	HeapBuffer<uint8_t> blob(sizeof(sys64_task_state_t));
	sys64_task_state_t* state = reinterpret_cast<sys64_task_state_t*>(&blob);

	// Integer Registers
	state->rax = 0;
	state->rbx = 0;
	state->rcx = 0;
	state->rdx = 0;
	state->rdi = 0;
	state->rsi = 0;
	state->r8 = 0;
	state->r9 = 0;
	state->r10 = 0;
	state->r11 = 0;
	state->r12 = 0;
	state->r13 = 0;
	state->r14 = 0;
	state->r15 = 0;
	
	// Control Registers
	state->rip = reinterpret_cast<sys64_addr_t>(entrypoint);
	state->rbp = reinterpret_cast<sys64_addr_t>(stackpointer);
	state->rsp = reinterpret_cast<sys64_addr_t>(stackpointer);

	return std::make_unique<TaskState>(std::move(blob));
}

//-----------------------------------------------------------------------------
// TaskState::Create<ProcessClass::x86_64> (static, private)
//
// Constructs a TaskState from an existing task state blob
//
// Arguments:
//
//	existing		- Pointer to the existing task state blob
//	length			- Length of the existing task state blob

template <>
std::unique_ptr<TaskState> TaskState::Create<ProcessClass::x86_64>(const void* existing, size_t length)
{
	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(sys64_task_state_t)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(sys64_task_state_t));

	// Copy the existing task information into a new HeapBuffer<> instance
	HeapBuffer<uint8_t> blob(sizeof(sys64_task_state_t));
	memcpy(blob, existing, length);

	return std::make_unique<TaskState>(std::move(blob));
}

#endif	// _M_X64

//-----------------------------------------------------------------------------
// TaskState::Create (static)
//
// Constructs a new TaskState for the provided process class
//
// Arguments:
//
//	_class			- Process class code (x86 / x86_64)
//	entrypoint		- Entry point for the new process or thread
//	stackpointer	- Stack pointer for the new process or thread

std::unique_ptr<TaskState> TaskState::Create(ProcessClass _class, const void* entrypoint, const void* stackpointer)
{
	// Select the correct internal function based on the process class
	switch(_class) {

		// x86 - sys32_task_state_t
		case ProcessClass::x86: return TaskState::Create<ProcessClass::x86>(entrypoint, stackpointer);

#ifdef _M_X64
		// x86_64 - sys64_task_state_t
		case ProcessClass::x86_64: return TaskState::Create<ProcessClass::x86_64>(entrypoint, stackpointer);
#endif
	}

	throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(_class));
}

//-----------------------------------------------------------------------------
// TaskState::Create (static)
//
// Constructs a new TaskState for the provided process class
//
// Arguments:
//
//	_class			- Process class code (x86 / x86_64)
//	existing		- Pointer to the existing blob of task state
//	length			- Length of the existing blob of task state

std::unique_ptr<TaskState> TaskState::Create(ProcessClass _class, const void* existing, size_t length)
{
	// Select the correct internal function based on the process class
	switch(_class) {

		// x86 - sys32_task_state_t
		case ProcessClass::x86: return TaskState::Create<ProcessClass::x86>(existing, length);

#ifdef _M_X64
		// x86_64 - sys64_task_state_t
		case ProcessClass::x86_64: return TaskState::Create<ProcessClass::x86_64>(existing, length);
#endif
	}

	throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(_class));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
