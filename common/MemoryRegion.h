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

#include <memory>
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// MemoryRegion
//
// Manages a region of virtual memory that is automatically released during
// instance destruction

class MemoryRegion
{
public:

	// Destructor
	//
	~MemoryRegion();

	//-------------------------------------------------------------------------
	// Member Functions

	// Commit
	//
	// Commits page(s) within the region
	void Commit(void* address, size_t length, uint32_t protect);

	// Decommit
	//
	// Decommits page(s) within the region
	void Decommit(void* address, size_t length);

	// Detach
	//
	// Detaches the memory region from the class instance
	void* Detach(void) { return Detach(nullptr); }
	void* Detach(PMEMORY_BASIC_INFORMATION meminfo);

	// Protect
	//
	// Applies protection flags to page(s) within the region
	uint32_t Protect(void* address, size_t length, uint32_t protect);

	// Reserve (static)
	//
	// Reserves a range of virtual memory; can also commit at the same time by specifying
	// MEM_COMMIT to an overload that accepts a flags argument
	static std::unique_ptr<MemoryRegion> Reserve(size_t length)
		{ return Reserve(INVALID_HANDLE_VALUE, length, nullptr, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(size_t length, uint32_t flags)
		{ return Reserve(INVALID_HANDLE_VALUE, length, nullptr, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(size_t length, void* address)
		{ return Reserve(INVALID_HANDLE_VALUE, length, address, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(size_t length, void* address, uint32_t flags)
		{ return Reserve(INVALID_HANDLE_VALUE, length, address, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length)
		{ return Reserve(process, length, nullptr, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, uint32_t flags)
		{ return Reserve(process, length, nullptr, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, void* address)
		{ return Reserve(process, length, address, MEM_RESERVE, PAGE_NOACCESS); }

	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, void* address, uint32_t flags)
		{ return Reserve(process, length, address, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

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

	MemoryRegion(const MemoryRegion&)=delete;
	MemoryRegion& operator=(const MemoryRegion&)=delete;

	// Instance Constructor
	//
	MemoryRegion(HANDLE process, void* base, size_t length, MEMORY_BASIC_INFORMATION& meminfo) : m_process(process), m_base(base), m_length(length), m_meminfo(meminfo) {}
	friend std::unique_ptr<MemoryRegion> std::make_unique<MemoryRegion, HANDLE&, void*&, size_t&, MEMORY_BASIC_INFORMATION&>(HANDLE&, void*&, size_t&, MEMORY_BASIC_INFORMATION&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// Reserve
	//
	// Reserves a range of virtual memory
	static std::unique_ptr<MemoryRegion> Reserve(HANDLE process, size_t length, void* address, uint32_t flags, uint32_t protect);

	//-------------------------------------------------------------------------
	// Member Variables

	void*						m_base;			// Base pointer for the memory region
	size_t						m_length;		// Length of the memory region
	HANDLE						m_process;		// Process to operate against
	MEMORY_BASIC_INFORMATION	m_meminfo;		// Actual allocation information
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MEMORYREGION_H_
