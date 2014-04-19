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

#include "Exception.h"
#include "Win32Exception.h"

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

	// AlignToAllocationGranularity
	//
	// Aligns an address down to the allocation granularity
	static void* AlignToAllocationGranularity(void* address);

	// AlignToPageSize
	//
	// Aligns an address down to the system page size
	static void* AlignToPageSize(void* address);

	// Commit
	//
	// Commits page(s) within the region
	void* Commit(void* address, size_t length, uint32_t protect);

	// Decommit
	//
	// Decommits page(s) within the region
	void* Decommit(void* address, size_t length);

	// Lock
	//
	// Locks page(s) within the region into physical memory
	void* Lock(void* address, size_t length);

	// Protect
	//
	// Applies protection flags to page(s) within the region
	void* Protect(void* address, size_t length, uint32_t protect);

	// Reserve
	//
	// Reserves a range of virtual memory
	static MemoryRegion* Reserve(size_t length)
		{ return new MemoryRegion(NULL, length, MEM_RESERVE, PAGE_NOACCESS); }

	static MemoryRegion* Reserve(size_t length, uint32_t flags)
		{ return new MemoryRegion(NULL, length, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	static MemoryRegion* Reserve(void* address, size_t length)
		{ return new MemoryRegion(address, length, MEM_RESERVE, PAGE_NOACCESS); }

	static MemoryRegion* Reserve(void* address, size_t length, uint32_t flags)
		{ return new MemoryRegion(address, length, MEM_RESERVE | flags, (flags & MEM_COMMIT) ? PAGE_READWRITE : PAGE_NOACCESS); }

	// Unlock
	//
	// Unlocks page(s) within the region so that they can be swapped out
	void* Unlock(void* address, size_t length);

	//-------------------------------------------------------------------------
	// Fields

	// AllocationGranularity
	//
	// Exposes the system allocation granularity
	static const size_t AllocationGranularity;

	// PageSize
	//
	// Exposes the system page size
	static const size_t PageSize;

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
	MemoryRegion(void* base, size_t length, uint32_t flags, uint32_t protect);

	// SystemInfo
	//
	// Used to initialize a static SYSTEM_INFO structure
	struct SystemInfo : public SYSTEM_INFO
	{
		SystemInfo() { GetNativeSystemInfo(static_cast<SYSTEM_INFO*>(this)); }
	};

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AlignDown/AlignUp
	//
	// Address alignment helper functions
	static uintptr_t AlignDown(uintptr_t address, size_t alignment);
	static uintptr_t AlignUp(uintptr_t address, size_t alignment);

	//-------------------------------------------------------------------------
	// Member Variables

	void*				m_base;				// Base pointer for the memory region
	size_t				m_length;			// Length of the memory region
	static SystemInfo	s_sysinfo;			// System information class
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __MEMORYREGION_H_
