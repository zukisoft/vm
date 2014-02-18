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

#include "stdafx.h"						// Include project pre-compiled headers
#include "MemoryRegion.h"				// Include MemoryRegion declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

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

MemoryRegion::MemoryRegion(void* base, size_t length, DWORD flags, DWORD protect)
{
	// Verify that the specified address aligns with the allocation granularity
	if(base && (intptr_t(base) % AllocationGranularity)) throw Win32Exception(ERROR_MAPPED_ALIGNMENT);

	// Pass the arguments onto VirtualAlloc() and just throw any resultant error
	m_base = VirtualAlloc(base, length, flags, protect);
	if(!m_base) throw Win32Exception();

	m_length = length;					// Save the length for destructor
}

//-----------------------------------------------------------------------------
// MemoryRegion Destructor

MemoryRegion::~MemoryRegion()
{
	if(m_base) VirtualFree(m_base, m_length, MEM_RELEASE);
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

void MemoryRegion::Protect(intptr_t address, size_t length, DWORD protect)
{
	DWORD		oldprotect;					// Old protection flags

	intptr_t base = intptr_t(m_base);
	if((address < base) || (address > (base + intptr_t(m_length)))) throw Exception(E_BOUNDS);

	// Verify that the specified address aligns to a page boundary
	if(address % s_sysinfo.dwPageSize) throw Win32Exception(ERROR_MAPPED_ALIGNMENT);

	// Apply the requested protection flags; throw away the rest
	if(!VirtualProtect(reinterpret_cast<void*>(address), length, protect, &oldprotect))
		throw Win32Exception();
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
