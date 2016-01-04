//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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
// MemoryRegion Destructor

MemoryRegion::~MemoryRegion()
{
	// If the region has not been detached, release it with VirtualFreeEx()
	if(m_base) VirtualFreeEx(m_process, m_meminfo.AllocationBase, 0, MEM_RELEASE);
}

//-----------------------------------------------------------------------------
// MemoryRegion::Commit
//
// Commits page(s) of memory within the region using the specified protection
//
// Arguments:
//
//	address		- Base address to be committed
//	length		- Length of the region to be committed
//	protect		- Protection flags to be applied to the committed region

void MemoryRegion::Commit(void* address, size_t length, uint32_t protect)
{
	// The system will automatically align the provided address downward and
	// adjust the length such that entire page(s) will be committed
	if(VirtualAllocEx(m_process, address, length, MEM_COMMIT, protect) == nullptr) throw Win32Exception();
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

void MemoryRegion::Decommit(void* address, size_t length)
{
	// The system will automatically align the provided address downward and
	// adjust the length such that entire page(s) will be decommitted
	if(!VirtualFreeEx(m_process, address, length, MEM_DECOMMIT)) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// MemoryRegion::Detach
//
// Detaches the memory region from the class so that it will not be released
// when the memory region instance is destroyed.
//
// Arguments:
//
//	meminfo		- MEMORY_BASIC_INFORMATION pointer to receive region data
//	length		- Optional size_t pointer to receive the region length
//
// NOTE: Returns the base pointer originally set up by Reserve(); the base
// allocation pointer can be accessed via the MEMORY_BASIC_INFORMATION

void* MemoryRegion::Detach(PMEMORY_BASIC_INFORMATION meminfo)
{
	// Copy the data regarding the entire allocated region if requested
	if(meminfo != nullptr) *meminfo = m_meminfo;

	void* base = m_base;			// Save original pointer from Reserve()

	// Reset member variables to an uninitialized state to prevent use
	m_base = nullptr;
	m_length = 0;
	m_process = INVALID_HANDLE_VALUE;
	memset(&m_meminfo, 0, sizeof(MEMORY_BASIC_INFORMATION));

	return base;					// Return the original base pointer
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

uint32_t MemoryRegion::Protect(void* address, size_t length, uint32_t protect)
{
	DWORD				oldprotect;			// Previous protection flags

	// The system will automatically align the provided address downward and
	// adjust the length such that entire page(s) will be decommitted
	if(!VirtualProtectEx(m_process, address, length, protect, &oldprotect)) throw Win32Exception();
	return oldprotect;
}

//-----------------------------------------------------------------------------
// MemoryRegion::Reserve (private, static)
//
// Reserves (and optionally commits) a region of virtual memory.  When an address
// has been specified, the system will attempt to construct a region that is aligned
// down to the proper boundary that contains the requested address and length
//
// Arguments:
//
//	process		- Optional process handle to operate against or INVALID_HANDLE_VALUE
//	address		- Optional base address to use for the allocation
//	length		- Length of the memory region to allocate
//	flags		- Memory region allocation type flags
//	protect		- Memory region protection flags

std::unique_ptr<MemoryRegion> MemoryRegion::Reserve(HANDLE process, size_t length, void* address, uint32_t flags, uint32_t protect)
{
	// Map INVALID_HANDLE_VALUE to the current process handle so that the Ex() version of the
	// virtual memory API functions can be used exclusively by the MemoryRegion class instance
	if(process == INVALID_HANDLE_VALUE) process = GetCurrentProcess();

	// Attempt to reserve a memory region large enough to hold the requested length.  If an
	// address was specified, it will be automatically rounded down by the system as needed
	void* regionbase = VirtualAllocEx(process, address, length, flags, protect);
	if(regionbase == nullptr) throw Win32Exception();

	// Query to determine the resultant memory region after adjustment by the system
	MEMORY_BASIC_INFORMATION meminfo;
	if(VirtualQueryEx(process, regionbase, &meminfo, sizeof(MEMORY_BASIC_INFORMATION)) == 0) {

		Win32Exception exception;									// Save exception code
		VirtualFreeEx(process, regionbase, 0, MEM_RELEASE);			// Release the region
		throw exception;											
	}
	
	// Region has been successfully reserved using the provided parameters
	return std::make_unique<MemoryRegion>(process, (address) ? address : regionbase, length, meminfo);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
