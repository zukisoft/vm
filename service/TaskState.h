//-----------------------------------------------------------------------------
// Copyright (c) 2014 Michael G. Brehm
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
#include "ElfClass.h"
#include "Exception.h"
#include "HeapBuffer.h"

#pragma warning(push, 4)				
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// TaskState
//
// Class that abstracts the architecture specific task state structures into a 
// blob that can be maintained by a Process object

class TaskState
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// CopyTo
	//
	// Copies the task state blob, the size must match exactly
	void CopyTo(void* taskstate, size_t length);

	// Create (static)
	//
	// Creates a new TaskState blob for the specified process class
	static std::unique_ptr<TaskState> Create(ElfClass _class, void* entrypoint, void* stackpointer);

	// Create (static)
	//
	// Creates a new TaskState blob from an existing task state blob
	static std::unique_ptr<TaskState> Create(ElfClass _class, void* existing, size_t length);

private:

	TaskState(const TaskState&)=delete;
	TaskState& operator=(const TaskState&)=delete;

	// Instance Constructor
	//
	TaskState(HeapBuffer<uint8_t>&& blob) : m_blob(std::move(blob)) {}
	friend std::unique_ptr<TaskState> std::make_unique<TaskState, HeapBuffer<uint8_t>>(HeapBuffer<uint8_t>&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Create (static)
	//
	// Creates a new TaskState blob with the provided entry and stack pointers
	template <ElfClass _class> 
	static std::unique_ptr<TaskState> Create(void* entrypoint, void* stackpointer);

	// Create (static)
	//
	// Creates a new TaskState blob from an existing task state blob
	template <ElfClass _class> 
	static std::unique_ptr<TaskState> Create(void* existing, size_t length);

	//-------------------------------------------------------------------------
	// Member Variables
	
	const HeapBuffer<uint8_t>		m_blob;			// Contained blob of data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __TASKSTATE_H_
