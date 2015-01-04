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

#include "stdafx.h"
#include "MemorySection.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// MemorySection Move Constructor (protected)

MemorySection::MemorySection(std::unique_ptr<MemorySection>&& rhs) : m_process(rhs->m_process), m_section(rhs->m_section), 
	m_address(rhs->m_address), m_length(rhs->m_length)
{
	// Nullify the member variables of the source instance
	rhs->m_process = nullptr;
	rhs->m_section = nullptr;
	rhs->m_address = nullptr;
	rhs->m_length = 0;
}

//-----------------------------------------------------------------------------
// MemorySection Destructor

MemorySection::~MemorySection()
{
	// Unmap the section view from the process and close the section handle
	if(m_address) NtApi::NtUnmapViewOfSection(m_process, m_address);
	if(m_section) NtApi::NtClose(m_section);
}

//-----------------------------------------------------------------------------
// MemorySection::Clone
//
// Clones this memory section into another process
//
// Arguments:
//
//	process			- Target process handle
//	mode			- Cloning mode (shared, copy-on-write, duplicate)

std::unique_ptr<MemorySection> MemorySection::Clone(HANDLE process, CloneMode mode)
{
	HANDLE						section;				// Duplicated section handle
	void*						mapbase = m_address;	// Same mapping address
	SIZE_T						maplength = 0;			// Same mapping length
	MEMORY_BASIC_INFORMATION	meminfo;				// Virtual memory region information
	ULONG						previous;				// Previously set protection flags
	NTSTATUS					result;					// Result from function call

	// Duplication of the section is best served in its own function
	if(mode == CloneMode::Duplicate) return Duplicate(process);

	// Set the new section handle's access mask based on the cloning mode
	ACCESS_MASK mask = (mode == CloneMode::SharedCopyOnWrite) ? 
		STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_EXECUTE : SECTION_ALL_ACCESS;

	// Duplicate the section handle with the same attributes as the original and the calculated access mask
	result = NtApi::NtDuplicateObject(NtApi::NtCurrentProcess, m_section, NtApi::NtCurrentProcess, &section, mask, 0, NtApi::DUPLICATE_SAME_ATTRIBUTES);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	try {

		// Attempt to map the original section into the target process, enabling copy-on-write as necessary
		ULONG prot = (mode == CloneMode::SharedCopyOnWrite) ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE;
		result = NtApi::NtMapViewOfSection(section, process, &mapbase, 0, 0, nullptr, &maplength, NtApi::ViewUnmap, 0, prot);
		if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

		try {

			uintptr_t begin = uintptr_t(m_address);
			uintptr_t end = begin + m_length;

			// Iterate over the source section to apply the same protection flags to the cloned section
			while(begin < end) {

				// Get information about the current region in the source section
				if(!VirtualQueryEx(m_process, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

				// If this region was committed in the source section, it will have been committed in the new mapping
				if(meminfo.State == MEM_COMMIT) {

					// If the cloning mode is copy-on-write, READWRITE protections must be adjusted to WRITECOPY protections
					if(mode == CloneMode::SharedCopyOnWrite) {

						if(meminfo.Protect == PAGE_READWRITE) meminfo.Protect = PAGE_WRITECOPY;
						else if(meminfo.Protect == PAGE_EXECUTE_READWRITE) meminfo.Protect = PAGE_EXECUTE_WRITECOPY;
					}

					// Apply the original protection flags to the auto-committed region in the cloned mapping
					result = NtApi::NtProtectVirtualMemory(process, &meminfo.BaseAddress, &meminfo.RegionSize, meminfo.Protect, &previous);
					if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);
				}

				begin += meminfo.RegionSize;				// Move to the next region in the source section
			}
		}

		catch(...) { NtApi::NtUnmapViewOfSection(process, mapbase); throw; }
	}

	catch(...) { NtApi::NtClose(section); throw; }

	// Construct the cloned MemorySection instance with the new handle, address and length
	size_t length = maplength;
	return std::make_unique<MemorySection>(process, section, mapbase, length);
}

//-----------------------------------------------------------------------------
// MemorySection::Commit
//
// Commits page(s) of memory within the section using the specified protection
//
// Arguments:
//
//	address		- Base address of the region to be committed
//	length		- Length of the region to be committed
//	protect		- Protection flags to be applied to the committed region

void MemorySection::Commit(void* address, size_t length, uint32_t protect)
{
	MEMORY_BASIC_INFORMATION		meminfo;		// Memory region information

	// The base allocation flags need to be determined to know if this was copy-on-write
	if(!VirtualQueryEx(m_process, address, &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

	// If this region was allocated for copy-on-write, the input flags may need to be adjusted
	if((meminfo.AllocationProtect == PAGE_WRITECOPY) || (meminfo.AllocationProtect == PAGE_EXECUTE_WRITECOPY)) {

		if(protect == PAGE_READWRITE) protect = PAGE_WRITECOPY;
		else if(protect == PAGE_EXECUTE_READWRITE) protect = PAGE_EXECUTE_WRITECOPY;
	}

	// The system will automatically align the provided address and length to page boundaries
	NTSTATUS result = NtApi::NtAllocateVirtualMemory(m_process, &address, 0, reinterpret_cast<PSIZE_T>(&length), MEM_COMMIT, protect);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// MemorySection::Duplicate (private)
//
// Duplicates this memory section into another process
//
// Arguments:
//
//	process		- Target process handle

std::unique_ptr<MemorySection> MemorySection::Duplicate(HANDLE process)
{
	void*						localaddress = nullptr;		// Local in-process mapping to copy from
	SIZE_T						locallength = 0;			// Local mapping length
	MEMORY_BASIC_INFORMATION	meminfo;					// Virtual memory region information
	NTSTATUS					result;						// Result from function call

	// Create a new section reservation with the same address and length as the original
	std::unique_ptr<MemorySection> duplicate = Reserve(process, m_address, m_length);

	// Map the section into the current process as READONLY so that the data can be copied into the new section
	result = NtApi::NtMapViewOfSection(m_section, NtApi::NtCurrentProcess, &localaddress, 0, 0, nullptr, &locallength, NtApi::ViewUnmap, 0, PAGE_READONLY);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	try {

		uintptr_t begin = uintptr_t(m_address);
		uintptr_t end = begin + m_length;

		// Iterate over the source section to apply the same status and protection flags to the new section
		while(begin < end) {

			// Get information about the current region within the source section
			if(!VirtualQueryEx(m_process, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

			// If the region is committed, copy the data from the source region to the destination region
			if(meminfo.State == MEM_COMMIT) {

				// Commit the region with READWRITE access first so that the data can be copied
				duplicate->Commit(meminfo.BaseAddress, meminfo.RegionSize, PAGE_READWRITE);

				// Use NtWriteVirtualMemory to copy the region from the local mapping into the duplicate mapping
				void* sourceaddress = reinterpret_cast<void*>(uintptr_t(localaddress) + (uintptr_t(meminfo.BaseAddress) - uintptr_t(m_address)));
				result = NtApi::NtWriteVirtualMemory(process, meminfo.BaseAddress, sourceaddress, meminfo.RegionSize, nullptr);
				if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

				// Apply the source region protection flags to the destination region
				duplicate->Protect(meminfo.BaseAddress, meminfo.RegionSize, meminfo.Protect);
			}

			begin += meminfo.RegionSize;							// Move to the next region within the original section
		}
	}

	catch(...) { NtApi::NtUnmapViewOfSection(NtApi::NtCurrentProcess, localaddress); throw; }

	NtApi::NtUnmapViewOfSection(NtApi::NtCurrentProcess, localaddress);		// Remove the local process mapping

	return duplicate;
}

//-----------------------------------------------------------------------------
// MemorySection::Protect
//
// Applies new protection flags to page(s) within the section
//
// Arguments:
//
//	address		- Base address to apply the protection
//	length		- Length of the memory to apply the protection to
//	protect		- Virtual memory protection flags

uint32_t MemorySection::Protect(void* address, size_t length, uint32_t protect)
{
	MEMORY_BASIC_INFORMATION	meminfo;		// Memory region information
	ULONG						previous;		// Previously set protection flags

	// The base allocation flags need to be determined to know if this was copy-on-write
	if(!VirtualQueryEx(m_process, address, &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

	// If this region was allocated for copy-on-write, the input flags may need to be adjusted
	if((meminfo.AllocationProtect == PAGE_WRITECOPY) || (meminfo.AllocationProtect == PAGE_EXECUTE_WRITECOPY)) {

		if(protect == PAGE_READWRITE) protect = PAGE_WRITECOPY;
		else if(protect == PAGE_EXECUTE_READWRITE) protect = PAGE_EXECUTE_WRITECOPY;
	}

	// The system will automatically align the provided address and length to page boundaries
	NTSTATUS result = NtApi::NtProtectVirtualMemory(m_process, &address, reinterpret_cast<PSIZE_T>(&length), protect, &previous);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	return previous;
}

//-----------------------------------------------------------------------------
// MemorySection::Reserve (static)
//
// Creates and reserves a memory section in the target process
//
// Arguments:
//
//	process			- Target process handle
//	address			- [Optional] specific address for the mapping
//	length			- Length of the section to create
//	granularity		- Granularity to assign to the length of the section
//	mapflags		- Mapping operation flags (limited to MEM_TOP_DOWN and MEM_LARGE_PAGES)

std::unique_ptr<MemorySection> MemorySection::Reserve(HANDLE process, void* address, size_t length, size_t granularity, int mapflags)
{
	HANDLE					section;			// Section handle
	LARGE_INTEGER			sectionlength;		// Section length
	SIZE_T					maplength = 0;		// Mapping length
	NTSTATUS				result;				// Result from function call

	// Verify that no invalid flags have been specified
	if(mapflags & ~(MEM_TOP_DOWN | MEM_LARGE_PAGES)) throw Win32Exception(ERROR_INVALID_PARAMETER);

	// When a zero has been specified for the granularity, use the system page size
	if(granularity == 0) granularity = SystemInformation::PageSize;

	// Align the requested address down to an allocation boundary and adjust length appropriately
	void* mapbase = align::down(address, SystemInformation::AllocationGranularity);
	length = align::up(length + (uintptr_t(address) - uintptr_t(mapbase)), granularity);

	// Allocate the section with PAGE_EXECUTE_READWRITE to allow the same protection when the section is mapped
	sectionlength.QuadPart = length;
	result = NtApi::NtCreateSection(&section, SECTION_ALL_ACCESS, nullptr, &sectionlength, PAGE_EXECUTE_READWRITE, SEC_RESERVE, nullptr);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	try {

		// Attempt to map the section into the target process' address space with PAGE_EXECUTE_READWRITE base protection
		result = NtApi::NtMapViewOfSection(section, process, &mapbase, 0, 0, nullptr, &maplength, NtApi::ViewUnmap, mapflags, PAGE_EXECUTE_READWRITE);
		if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

		_ASSERTE(maplength == length);					// Ensure the entire section was mapped
	}

	catch(...) { NtApi::NtClose(section); throw; }

	// Construct the MemorySection instance with the new section attributes
	return std::make_unique<MemorySection>(process, section, mapbase, length);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
