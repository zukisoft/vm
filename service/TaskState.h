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

#ifndef __TASKSTATE_H_
#define __TASKSTATE_H_
#pragma once

#include <memory>
#include "Architecture.h"
#include "Exception.h"
#include "Win32Exception.h"

#pragma warning(push, 4)				
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// TaskState
//
// Class that abstracts the architecture specific task state structures

class TaskState
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// Capture (static)
	//
	// Captures a TaskState from a suspended native thread
	static std::unique_ptr<TaskState> Capture(::Architecture architecture, HANDLE nativethread);

	// Create (static)
	//
	// Creates a new TaskState for the specified architecture
	template<::Architecture architecture>
	static std::unique_ptr<TaskState> Create(const void* entrypoint, const void* stackpointer);

	// Duplicate (static)
	//
	// Duplicates an existing TaskState instance
	static std::unique_ptr<TaskState> Duplicate(const std::unique_ptr<TaskState>& existing);

	// FromExisting (static)
	//
	// Creates a TaskState from an existing task state
	template <::Architecture architecture>
	static std::unique_ptr<TaskState> FromExisting(const void* existing, size_t length);

	// Restore
	//
	// Restores this task state to a suspended native thread
	void Restore(::Architecture architecture, HANDLE nativehandle) const;

	//-------------------------------------------------------------------------
	// Properties

	// Architecture
	//
	// Gets the architecture associated with the task state
	__declspec(property(get=getArchitecture)) ::Architecture Architecture;
	::Architecture getArchitecture(void) const;

	// Data
	//
	// Gets a pointer to the contained task state
	__declspec(property(get=getData)) const void* Data;
	const void* getData(void) const;

	// InstructionPointer
	//
	// Gets/sets the instruction pointer contained by the task state
	__declspec(property(get=getInstructionPointer, put=putInstructionPointer)) const void* InstructionPointer;
	const void* getInstructionPointer(void) const;
	void putInstructionPointer(const void* value);

	// Length
	//
	// Gets the length of the contained task state
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const;

	// ReturnValue
	//
	// Gets/sets the return value register contained by the task state
	__declspec(property(get=getReturnValue, put=putReturnValue)) unsigned __int3264 ReturnValue;
	unsigned __int3264 getReturnValue(void) const;
	void putReturnValue(unsigned __int3264 value);

	// StackPointer
	//
	// Gets/sets the stack pointer contained by the task state
	__declspec(property(get=getStackPointer, put=putStackPointer)) const void* StackPointer;
	const void* getStackPointer(void) const;
	void putStackPointer(const void* value);

private:

	TaskState(const TaskState&)=delete;
	TaskState& operator=(const TaskState&)=delete;

	// context32_t
	//
	// Structure used to hold a 32-bit thread context
#ifndef _M_X64
	using context32_t = CONTEXT;
#else
	using context32_t = WOW64_CONTEXT;
#endif

	// context64_t
	//
	// Structure used to hold a 64-bit thread context
#ifdef _M_X64
	using context64_t = CONTEXT;
#endif

	// context_t
	//
	// Union of the available context structures to store the task state
	union context_t {

		context32_t			x86;			// 32-bit context
#ifdef _M_X64
		context64_t			x86_64;			// 64-bit context
#endif
	};

	// Instance Constructor
	//
	TaskState(::Architecture architecture, context_t&& context) : m_architecture(std::move(architecture)), m_context(std::move(context)) {}
	friend std::unique_ptr<TaskState> std::make_unique<TaskState, ::Architecture, context_t>(::Architecture&&, context_t&&);

	//-------------------------------------------------------------------------
	// Member Variables
	
	::Architecture			m_architecture;			// Selected architecture
	context_t				m_context;				// Contained context data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TASKSTATE_H_
