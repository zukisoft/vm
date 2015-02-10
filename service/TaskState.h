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
#include <linux/ptrace.h>
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

	// CopyTo
	//
	// Copies the task state, the size must match exactly
	void CopyTo(void* taskstate, size_t length) const;

	// Create (static)
	//
	// Creates a new TaskState for the specified architecture
	static std::unique_ptr<TaskState> Create(Architecture architecture, const void* entrypoint, const void* stackpointer);

	// Create (static)
	//
	// Creates a new TaskState from an existing task state
	static std::unique_ptr<TaskState> Create(Architecture architecture, const void* existing, size_t length);

	// FromNativeThread (static)
	//
	// Captures the task state of a native operating system thread
	static std::unique_ptr<TaskState> FromNativeThread(Architecture architecture, HANDLE nativehandle);

	// ToNativeThread
	//
	// Applies this task state to a native operating system thread
	void ToNativeThread(Architecture architecture, HANDLE nativehandle);

private:

	TaskState(const TaskState&)=delete;
	TaskState& operator=(const TaskState&)=delete;

	// pt_regs_t
	//
	// Union of the available register sets to store for the task state
	union pt_regs_t {

		uapi::pt_regs32		x86;				// 32-bit register set
#ifdef _M_X64
		uapi::pt_regs64		x86_64;				// 64-bit register set
#endif
	};

	// Instance Constructor
	//
	TaskState(Architecture architecture, pt_regs_t&& regs) : m_architecture(std::move(architecture)), m_regs(std::move(regs)) {}
	friend std::unique_ptr<TaskState> std::make_unique<TaskState, Architecture, pt_regs_t>(Architecture&&, pt_regs_t&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Create (static)
	//
	// Creates a new TaskState blob with the provided entry and stack pointers
	template <Architecture architecture> 
	static std::unique_ptr<TaskState> Create(const void* entrypoint, const void* stackpointer);

	// Create (static)
	//
	// Creates a new TaskState blob from an existing task state blob
	template <Architecture architecture> 
	static std::unique_ptr<TaskState> Create(const void* existing, size_t length);

	// FromNativeThread (static)
	//
	// Captures the task state from a native operating system thread
	template <Architecture architecture>
	static std::unique_ptr<TaskState> FromNativeThread(HANDLE nativehandle);

	// ToNativeThread
	//
	// Applies this task state to a native operating system thread
	template <Architecture architecture>
	void ToNativeThread(HANDLE nativehandle);

	//-------------------------------------------------------------------------
	// Member Variables
	
	Architecture			m_architecture;			// Selected architecture
	pt_regs_t				m_regs;					// Contained register data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TASKSTATE_H_
