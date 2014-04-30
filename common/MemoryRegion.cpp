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

#include "stdafx.h"
#include "MemoryRegion.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// Static Initializers

MemoryRegion::SystemInfo MemoryRegion::s_sysinfo;
size_t const MemoryRegion::AllocationGranularity = MemoryRegion::s_sysinfo.dwAllocationGranularity;
size_t const MemoryRegion::PageSize = MemoryRegion::s_sysinfo.dwPageSize;

//-----------------------------------------------------------------------------
// MemoryRegion Constructor (private)
//
// Arguments:
//
//	base		- Optional base address to use for the allocation
//	length		- Length of the memory region to allocate
//	flags		- Memory region allocation type flags
//	protect		- Memory region protection flags

MemoryRegion::MemoryRegion(void* base, size_t length, uint32_t flags, uint32_t protect)
{
	uintptr_t requested = uintptr_t(base);
	uintptr_t aligned = uintptr_t(AlignToAllocationGranularity(base));

	// Adjust the requested length to accomodate any downward alignment
	length += requested - aligned;

	// Pass the arguments onto VirtualAlloc() and just throw any resultant error
	m_base = VirtualAlloc(base, length, flags, protect);
	if(!m_base) throw Win32Exception();

	m_length = length;					// Save the length for destructor
}

//-----------------------------------------------------------------------------
// MemoryRegion Destructor

MemoryRegion::~MemoryRegion()
{
	if(m_base) VirtualFree(m_base, 0, MEM_RELEASE);
}

//-----------------------------------------------------------------------------
// MemoryRegion::AlignToAllocationGranularity (static)
//
// Aligns an address down to the system allocation granularity
//
// Arguments:
//
//	address		- Address to be aligned

void* MemoryRegion::AlignToAllocationGranularity(void* address)
{
	return reinterpret_cast<void*>(AlignDown(uintptr_t(address), MemoryRegion::AllocationGranularity));
}

//-----------------------------------------------------------------------------
// MemoryRegion::AlignToPageSize (static)
//
// Aligns an address down to the system page size
//
// Arguments:
//
//	address		- Address to be aligned

void* MemoryRegion::AlignToPageSize(void* address)
{
	return reinterpret_cast<void*>(AlignDown(uintptr_t(address), MemoryRegion::PageSize));
}

//-----------------------------------------------------------------------------
// MemoryRegion::AlignDown (private, static)
//
// Aligns an offset down to the specified alignment
//
// Arguments:
//
//	address		- Address to be aligned
//	alignment	- Alignment

uintptr_t MemoryRegion::AlignDown(uintptr_t address, size_t alignment)
{
	if(alignment < 1) throw Exception(E_ARGUMENTOUTOFRANGE, _T("alignment"));

	if(address < alignment) return 0;
	else return AlignUp(address - (alignment - 1), alignment);
}

//---------------------------------------------------------------------------
// MemoryRegion::AlignUp (private, static)
//
// Aligns an offset up to the specified alignment
//
// Arguments:
//
//	address		- Address to be aligned
//	alignment	- Alignment

uintptr_t MemoryRegion::AlignUp(uintptr_t address, size_t alignment)
{
	if(alignment < 1) throw Exception(E_ARGUMENTOUTOFRANGE, _T("alignment"));

	if(address == 0) return 0;
	else return address + ((alignment - (address % alignment)) % alignment);
}

//-----------------------------------------------------------------------------
// MemoryRegion::Commit
//
// Commits page(s) of memory within the region using the specified protection
// attributes.  If address does not align on a page boundary it will be aligned
// down and returned as the result from this function
//
// Arguments:
//
//	address		- Base address to be committed
//	length		- Length of the region to be committed
//	protect		- Protection flags to be applied to the committed region

void* MemoryRegion::Commit(void* address, size_t length, uint32_t protect)
{
	uintptr_t base = uintptr_t(m_base);
	uintptr_t requested = uintptr_t(address);
	uintptr_t aligned = uintptr_t(AlignToPageSize(address));

	// Verify the requested address space is not outside of the region
	length += requested - aligned;
	if((aligned < base) || ((aligned + length) > (base + m_length))) throw Exception(E_BOUNDS);	

	// Use VirtualAlloc() to commit the page(s) within the region
	return VirtualAlloc(reinterpret_cast<void*>(aligned), length, MEM_COMMIT, protect);
}

//-----------------------------------------------------------------------------
// MemoryRegion::Decommit
//
// Decommits page(s) of memory from within the region 
//
// Arguments:
//
//	address		- Base address to be decommitted
//	length		- Length of the region to be decommitted

void* MemoryRegion::Decommit(void* address, size_t length)
{
	uintptr_t base = uintptr_t(m_base);
	uintptr_t requested = uintptr_t(address);
	uintptr_t aligned = uintptr_t(AlignToPageSize(address));

	// Verify the requested address space is not outside of the region
	length += requested - aligned;
	if((aligned < base) || ((aligned + length) > (base + m_length))) throw Exception(E_BOUNDS);	

	// Use VirtualFree() to decommit the page(s) from within the region
	if(!VirtualFree(reinterpret_cast<void*>(aligned), length, MEM_DECOMMIT)) throw Win32Exception();

	return reinterpret_cast<void*>(aligned);
}

//-----------------------------------------------------------------------------
// MemoryRegion::Lock
//
// Locks page(s) of the region into physical memory
//
// Arguments:
//
//	address		- Base address to be locked
//	length		- Length of the region to be locked

void* MemoryRegion::Lock(void* address, size_t length)
{
	uintptr_t base = uintptr_t(m_base);
	uintptr_t requested = uintptr_t(address);
	uintptr_t aligned = uintptr_t(AlignToPageSize(address));

	// Verify the requested address space is not outside of the region
	length += requested - aligned;
	if((aligned < base) || ((aligned + length) > (base + m_length))) throw Exception(E_BOUNDS);	

	// Use VirtualLock() to lock the page(s) into physical memory
	if(!VirtualLock(reinterpret_cast<void*>(aligned), length)) throw Win32Exception();

	return reinterpret_cast<void*>(aligned);
}

//-----------------------------------------------------------------------------
// MemoryRegion::Protect
//
// Applies new protection flags to page(s) within the allocated region
//
// Arguments:
//
//	address		- Base address to apply the protection
//	length		- Length of the memory to apply the protection to
//	protect		- Virtual memory protection flags

void* MemoryRegion::Protect(void* address, size_t length, uint32_t protect)
{
	DWORD		oldprotect;					// Old protection flags

	uintptr_t base = uintptr_t(m_base);
	uintptr_t requested = uintptr_t(address);
	uintptr_t aligned = uintptr_t(AlignToPageSize(address));

	// Verify the requested address space is not outside of the region
	length += requested - aligned;
	if((aligned < base) || ((aligned + length) > (base + m_length))) throw Exception(E_BOUNDS);	

	// Apply the requested protection flags; throw away the rest
	if(!VirtualProtect(reinterpret_cast<void*>(aligned), length, protect, &oldprotect)) throw Win32Exception();

	return reinterpret_cast<void*>(aligned);
}

//-----------------------------------------------------------------------------
// MemoryRegion::Unlock
//
// Unlocks page(s) of the region from physical memory
//
// Arguments:
//
//	address		- Base address to be unlocked
//	length		- Length of the region to be unlocked

void* MemoryRegion::Unlock(void* address, size_t length)
{
	uintptr_t base = uintptr_t(m_base);
	uintptr_t requested = uintptr_t(address);
	uintptr_t aligned = uintptr_t(AlignToPageSize(address));

	// Verify the requested address space is not outside of the region
	length += requested - aligned;
	if((aligned < base) || ((aligned + length) > (base + m_length))) throw Exception(E_BOUNDS);	

	// Use VirtualUnlock() to unlock the page(s) from physical memory
	if(!VirtualUnlock(reinterpret_cast<void*>(aligned), length)) throw Win32Exception();

	return reinterpret_cast<void*>(aligned);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
