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

#ifndef __NATIVETHREAD_H_
#define __NATIVETHREAD_H_
#pragma once

#include <memory>
#include "Architecture.h"

#pragma warning(push, 4)				
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// NativeThread
//
//	words

class NativeThread
{
public:

	// Destructor
	//
	~NativeThread()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Capture (static)
	//
	// Captures a Task from a suspended native thread
	//static std::unique_ptr<Task> Capture(enum class Architecture architecture, HANDLE nativethread);

	// Create (static)
	//
	// Creates a new Task for the specified architecture
	static std::unique_ptr<NativeThread> Create(enum class Architecture architecture, uintptr_t instructionpointer, uintptr_t stackpointer);

	// Duplicate (static)
	//
	// Duplicates an existing Task instance
	//static std::unique_ptr<Task> Duplicate(std::unique_ptr<Task> const& existing);

	// FromExisting (static)
	//
	// Creates a Task from an existing task state
	//static std::unique_ptr<Task> FromExisting(void const* existing, size_t length);
	// todo: can FromExisting go away

	// Restore
	//
	// Restores this task state to a suspended native thread
	//void Restore(HANDLE nativehandle) const;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture associated with the task state
	__declspec(property(get=getArchitecture)) enum class Architecture Architecture;
	enum class Architecture getArchitecture(void) const;

	// Data
	//
	// Gets a pointer to the contained task state
	//__declspec(property(get=getData)) void const* Data;
	//void const* getData(void) const;

	// InstructionPointer
	//
	// Gets/sets the instruction pointer contained by the task state
	//__declspec(property(get=getInstructionPointer)) uintptr_t InstructionPointer;
	//uintptr_t getInstructionPointer(void) const;

	// Length
	//
	// Gets the length of the contained task state
	//__declspec(property(get=getLength)) size_t Length;
	//size_t getLength(void) const;

	// ReturnValue
	//
	// Gets/sets the return value register contained by the task state
	//__declspec(property(get=getReturnValue, put=putReturnValue)) unsigned __int3264 ReturnValue;
	//unsigned __int3264 getReturnValue(void) const;
	//void putReturnValue(unsigned __int3264 value);

	// StackPointer
	//
	// Gets/sets the stack pointer contained by the task state
	//__declspec(property(get=getStackPointer)) uintptr_t StackPointer;
	//uintptr_t getStackPointer(void) const;

private:

	NativeThread(NativeThread const&)=delete;
	NativeThread& operator=(NativeThread const&)=delete;

	// task_t
	//
	// Union of the available context structures to store the task state
	union task_t {

		uapi::utask32		x86;			// 32-bit context
#ifdef _M_X64
		uapi::utask64		x86_64;			// 64-bit context
#endif
	};

	// Windows API
	//
	using GetThreadContext32Func = BOOL(WINAPI*)(HANDLE, uapi::utask32*);
	using SetThreadContext32Func = BOOL(WINAPI*)(HANDLE, uapi::utask32 const*);
#ifdef _M_X64
	using GetThreadContext64Func = BOOL(WINAPI*)(HANDLE, uapi::utask64*);
	using SetThreadContext64Func = BOOL(WINAPI*)(HANDLE, uapi::utask64 const*);
#endif

	// Instance Constructor
	//
	NativeThread(enum class Architecture architecture, task_t&& task); //// : m_architecture(std::move(architecture)), m_task(std::move(task)) {}
	friend std::unique_ptr<NativeThread> std::make_unique<NativeThread, enum class Architecture, task_t>(enum class Architecture&&, task_t&&);

	//-------------------------------------------------------------------------
	// Private Member Functions
	
	// Create<Architecture>
	//
	// Architecture-specific implementation of Create()
	template<enum class Architecture architecture>
	static std::unique_ptr<NativeThread> Create(uintptr_t instructionpointer, uintptr_t stackpointer);

	//-------------------------------------------------------------------------
	// Member Variables
	
	enum class Architecture	const	m_architecture;		// Task architecture
	task_t							m_task;				// Contained task information

	// Windows API
	//
	static GetThreadContext32Func const GetThreadContext32;
	static SetThreadContext32Func const SetThreadContext32;
#ifdef _M_X64
	static GetThreadContext64Func const GetThreadContext64;
	static SetThreadContext64Func const SetThreadContext64;
#endif
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVETHREAD_H_
