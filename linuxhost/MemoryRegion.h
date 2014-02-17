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

#ifndef __MEMORYREGION_H_
#define __MEMORYREGION_H_
#pragma once

#include "Exception.h"					// Include Exception declarations
#include "Win32Exception.h"				// Include Win32Exception decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// MemoryRegion
//
// Wrapper class to contain a memory region allocated with VirtualAlloc so that
// an automatic destructor can be provided to release it

class MemoryRegion
{
public:

	// Destructor
	//
	~MemoryRegion();

	//-------------------------------------------------------------------------
	// Member Functions

	// Allocate
	//
	// Allocates the memory region
	static MemoryRegion* Allocate(size_t length, DWORD protect)
		{ return new MemoryRegion(NULL, length, MEM_RESERVE | MEM_COMMIT, protect); }

	static MemoryRegion* Allocate(size_t length, DWORD protect, void* base)
		{ return new MemoryRegion(base, length, MEM_RESERVE | MEM_COMMIT, protect); }

	static MemoryRegion* Allocate(size_t length, DWORD protect, DWORD flags)
		{ return new MemoryRegion(NULL, length, MEM_RESERVE | MEM_COMMIT | flags, protect); }

	static MemoryRegion* Allocate(size_t length, DWORD protect, DWORD flags, void* base)
		{ return new MemoryRegion(base, length, MEM_RESERVE | MEM_COMMIT | flags, protect); }

	//-------------------------------------------------------------------------
	// Properties

	// Length
	//
	// Gets the length of the memory region
	__declspec(property(get=getLength)) size_t Length;
	size_t getLength(void) const { return m_length; }

	// Pointer
	//
	// Gets the base pointer for the memory region
	__declspec(property(get=getPointer)) void* Pointer;
	void* getPointer(void) const { return m_base; }

private:

	MemoryRegion(const MemoryRegion&);
	MemoryRegion& operator=(const MemoryRegion&);

	// Instance Constructor
	//
	MemoryRegion(void* base, size_t length, DWORD flags, DWORD protect);

	//-------------------------------------------------------------------------
	// Member Variables

	void*				m_base;				// Base pointer for the memory region
	size_t				m_length;			// Length of the memory region
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MEMORYREGION_H_
