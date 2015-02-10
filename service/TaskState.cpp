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
	
	if((m_architecture == Architecture::x86) && (length != sizeof(uapi::pt_regs32))) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(uapi::pt_regs32));
#ifdef _M_X64
	if((m_architecture == Architecture::x86_64) && (length != sizeof(uapi::pt_regs64))) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(uapi::pt_regs64));
#endif

	// The size matches, just copy the data from the contained buffer
	memcpy(taskstate, &m_regs, length);
}

//-----------------------------------------------------------------------------
// TaskState::Create<Architecture::x86> (static, private)
//
// Constructs a TaskState for a new process or thread
//
// Arguments:
//
//	entrypoint		- Address of the process/thread entry point
//	stackpointer	- Address of the process/thread stack pointer

template <>
std::unique_ptr<TaskState> TaskState::Create<Architecture::x86>(const void* entrypoint, const void* stackpointer)
{
	pt_regs_t registers;

	// Integer Registers
	registers.x86.eax = 0;
	registers.x86.ebx = 0;
	registers.x86.ecx = 0;
	registers.x86.edx = 0;
	registers.x86.edi = 0;
	registers.x86.esi = 0;

	// Control Registers
	registers.x86.eip = reinterpret_cast<uint32_t>(entrypoint);
	registers.x86.ebp = reinterpret_cast<uint32_t>(stackpointer);
	registers.x86.esp = reinterpret_cast<uint32_t>(stackpointer);
	registers.x86.eflags = 0;

	// Segment Registers
	registers.x86.gs = 0;

	return std::make_unique<TaskState>(Architecture::x86, std::move(registers));
}

//-----------------------------------------------------------------------------
// TaskState::Create<Architecture::x86> (static, private)
//
// Constructs a TaskState from an existing task state blob
//
// Arguments:
//
//	existing		- Pointer to the existing task state blob
//	length			- Length of the existing task state blob

template <>
std::unique_ptr<TaskState> TaskState::Create<Architecture::x86>(const void* existing, size_t length)
{
	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(uapi::pt_regs32)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(uapi::pt_regs32));

	// Copy the existing task information
	pt_regs_t registers;
	memcpy(&registers.x86, existing, length);

	return std::make_unique<TaskState>(Architecture::x86, std::move(registers));
}

#ifdef _M_X64

//-----------------------------------------------------------------------------
// TaskState::Create<Architecture::x86_64> (static, private)
//
// Constructs a TaskState for a new process or thread
//
// Arguments:
//
//	entrypoint		- Address of the process/thread entry point
//	stackpointer	- Address of the process/thread stack pointer

template <>
std::unique_ptr<TaskState> TaskState::Create<Architecture::x86_64>(const void* entrypoint, const void* stackpointer)
{
	pt_regs_t registers;

	// Integer Registers
	registers.x86_64.rax = 0;
	registers.x86_64.rbx = 0;
	registers.x86_64.rcx = 0;
	registers.x86_64.rdx = 0;
	registers.x86_64.rdi = 0;
	registers.x86_64.rsi = 0;
	registers.x86_64.r8 = 0;
	registers.x86_64.r9 = 0;
	registers.x86_64.r10 = 0;
	registers.x86_64.r11 = 0;
	registers.x86_64.r12 = 0;
	registers.x86_64.r13 = 0;
	registers.x86_64.r14 = 0;
	registers.x86_64.r15 = 0;
	
	// Control Registers
	registers.x86_64.rip = reinterpret_cast<uint64_t>(entrypoint);
	registers.x86_64.rbp = reinterpret_cast<uint64_t>(stackpointer);
	registers.x86_64.rsp = reinterpret_cast<uint64_t>(stackpointer);
	registers.x86_64.rflags = 0;

	return std::make_unique<TaskState>(Architecture::x86_64, std::move(registers));
}

//-----------------------------------------------------------------------------
// TaskState::Create<Architecture::x86_64> (static, private)
//
// Constructs a TaskState from an existing task state blob
//
// Arguments:
//
//	existing		- Pointer to the existing task state blob
//	length			- Length of the existing task state blob

template <>
std::unique_ptr<TaskState> TaskState::Create<Architecture::x86_64>(const void* existing, size_t length)
{
	if(existing == nullptr) throw Exception(E_POINTER);
	if(length != sizeof(uapi::pt_regs64)) throw Exception(E_TASKSTATEINVALIDLENGTH, length, sizeof(uapi::pt_regs64));

	// Copy the existing task information
	pt_regs_t registers;
	memcpy(&registers.x86_64, existing, length);

	return std::make_unique<TaskState>(Architecture::x86_64, std::move(registers));
}

#endif	// _M_X64

//-----------------------------------------------------------------------------
// TaskState::Create (static)
//
// Constructs a new TaskState for the provided process class
//
// Arguments:
//
//	architecture	- Process architecture
//	entrypoint		- Entry point for the new process or thread
//	stackpointer	- Stack pointer for the new process or thread

std::unique_ptr<TaskState> TaskState::Create(Architecture architecture, const void* entrypoint, const void* stackpointer)
{
	// Select the correct internal function based on the process class
	switch(architecture) {

		// x86 - sys32_task_state_t
		case Architecture::x86: return TaskState::Create<Architecture::x86>(entrypoint, stackpointer);

#ifdef _M_X64
		// x86_64 - sys64_task_state_t
		case Architecture::x86_64: return TaskState::Create<Architecture::x86_64>(entrypoint, stackpointer);
#endif
	}

	throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(architecture));
}

//-----------------------------------------------------------------------------
// TaskState::Create (static)
//
// Constructs a new TaskState for the provided process class
//
// Arguments:
//
//	architecture	- Process architecture
//	existing		- Pointer to the existing blob of task state
//	length			- Length of the existing blob of task state

std::unique_ptr<TaskState> TaskState::Create(Architecture architecture, const void* existing, size_t length)
{
	// Select the correct internal function based on the process class
	switch(architecture) {

		// x86 - pt_regs32
		case Architecture::x86: return TaskState::Create<Architecture::x86>(existing, length);

#ifdef _M_X64
		// x86_64 - pt_regs64
		case Architecture::x86_64: return TaskState::Create<Architecture::x86_64>(existing, length);
#endif
	}

	throw Exception(E_TASKSTATEUNSUPPORTEDCLASS, static_cast<int>(architecture));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
