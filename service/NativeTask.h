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

#ifndef __NATIVETASK_H_
#define __NATIVETASK_H_
#pragma once

#include <memory>
#include "Exception.h"
#include "HeapBuffer.h"
#include "ProcessClass.h"

#pragma warning(push, 4)				
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// NativeTask
//
// Abstracts the architecture specific task state (CONTEXT) structures as blobs

class NativeTask
{
public:

//	// CONTEXT32 / CONTEXT64
////
//// Aliases for either CONTEXT or WOW64_CONTEXT, depending on build type
//#ifndef _M_X64
//using CONTEXT32 = CONTEXT;
//#define GetThreadContext32	GetThreadContext
//#define SetThreadContext32	SetThreadContext
//#else
//using CONTEXT32 = WOW64_CONTEXT;
//using CONTEXT64 = CONTEXT;
//#define GetThreadContext32	Wow64GetThreadContext
//#define GetThreadContext64	GetThreadContext
//#define SetThreadContext32	Wow64SetThreadContext
//#define SetThreadContext64	SetThreadContext
//#endif

	//-------------------------------------------------------------------------
	// Member Functions

	// CopyTo
	//
	// Copies the task state blob, the size must match exactly
	//void CopyTo(void* taskstate, size_t length) const;

	// FromThread (static)
	//
	// Generates a new task blob from an existing class-specific task structure
	template <typename _context>
	static std::unique_ptr<NativeTask> FromExisting(_context* context);

	//-------------------------------------------------------------------------
	// Properties

	// InstructionPointer
	//
	// Gets/sets the instruction pointer embedded in the context blob
	__declspec(property(get=getInstructionPointer, put=putInstructionPointer)) void* InstructionPointer;
	void* getInstructionPointer(void);
	void putInstructionPointer(void* value);

	// StackPointer
	//
	// Gets/sets the instruction pointer embedded in the context blob
	__declspec(property(get=getStackPointer, put=putStackPointer)) void* StackPointer;
	void* getStackPointer(void);
	void putStackPointer(void* value);

private:

	NativeTask(const NativeTask&)=delete;
	NativeTask& operator=(const NativeTask&)=delete;

	// Instance Constructor
	//
	NativeTask(ProcessClass _class, HeapBuffer<uint8_t>&& blob) : m_class(_class), m_blob(std::move(blob)) {}
	friend std::unique_ptr<NativeTask> std::make_unique<NativeTask, ProcessClass&, HeapBuffer<uint8_t>>(ProcessClass&, HeapBuffer<uint8_t>&&);

	//-------------------------------------------------------------------------
	// Member Variables
	
	const ProcessClass				m_class;		// Task structure format
	const HeapBuffer<uint8_t>		m_blob;			// Contained blob of data
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __NATIVETASK_H_
