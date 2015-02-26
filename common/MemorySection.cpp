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
// AdjustProtectionForMode (local)
//
// Modifies protection mode flags based on the current mode
//
// Arguments:
//
//	protection		- Protection flags to check/adjust based on mode

static inline uint32_t AdjustProtectionForMode(uint32_t protection, MemorySection::Mode mode)
{
	// Copy-on-write mode sections get READWRITE access swapped to WRITECOPY access
	if(mode == MemorySection::Mode::CopyOnWrite) {

		if(protection & PAGE_READWRITE) protection = ((protection & ~PAGE_READWRITE) | PAGE_WRITECOPY);
		else if(protection == PAGE_EXECUTE_READWRITE) protection = ((protection & ~PAGE_EXECUTE_READWRITE) | PAGE_EXECUTE_WRITECOPY);
	}

	// Private and shared sections get WRITECOPY access swapped to READWRITE access
	else {

		if(protection == PAGE_WRITECOPY) protection = ((protection & ~PAGE_WRITECOPY) | PAGE_READWRITE);
		else if(protection == PAGE_EXECUTE_WRITECOPY) protection = ((protection & ~PAGE_EXECUTE_WRITECOPY) | PAGE_EXECUTE_READWRITE);
	}

	return protection;
}

//-----------------------------------------------------------------------------
// MemorySection Constructor (private)
//
// Arguments:
//
//	process			- Target process handle
//	section			- Section object handle
//	baseaddress		- Base address of the mapping
//	length			- Length of the section
//	mode			- Initial section protection behavior mode

MemorySection::MemorySection(HANDLE process, HANDLE section, void* baseaddress, size_t length, Mode mode) :
	m_process(process), m_section(section), m_address(baseaddress), m_length(length), m_mode(mode), m_allocmap(length / SystemInformation::PageSize)
{
	// The length of the section should align to the system allocation granularity
	_ASSERTE((length % SystemInformation::AllocationGranularity) == 0);
}

//-----------------------------------------------------------------------------
// MemorySection Constructor (private)
//
// Arguments:
//
//	process			- Target process handle
//	section			- Section object handle
//	baseaddress		- Base address of the mapping
//	length			- Length of the section
//	mode			- Initial section protection behavior mode
//	bitmap			- Reference to an existing allocation bitmap to copy

MemorySection::MemorySection(HANDLE process, HANDLE section, void* baseaddress, size_t length, Mode mode, const Bitmap& bitmap) :
	m_process(process), m_section(section), m_address(baseaddress), m_length(length), m_mode(mode), m_allocmap(bitmap)
{
	// The length of the section should align to the system allocation granularity
	_ASSERTE((length % SystemInformation::AllocationGranularity) == 0);
}

//-----------------------------------------------------------------------------
// MemorySection Destructor

MemorySection::~MemorySection()
{
	// Unmap the view from the process address space and close the section handle
	if(m_address) NtApi::NtUnmapViewOfSection(m_process, m_address);
	if(m_section) NtApi::NtClose(m_section);
}

//-----------------------------------------------------------------------------
// MemorySection::Allocate
//
// Allocates (commits) pages within the memory section
//
// Arguments:
//
//	address			- Base address of the pages to be committed
//	length			- Length of the region to be committed
//	protection		- Protection flags to assign to the committed pages

void* MemorySection::Allocate(void* address, size_t length, uint32_t protection)
{
	// Verify that the requested range falls within this section's virtual address space
	if(address < m_address) throw Win32Exception(ERROR_INVALID_ADDRESS);
	if(uintptr_t(address) + length > uintptr_t(m_address) + m_length) throw Win32Exception(ERROR_INVALID_ADDRESS);

	// Make any necessary changes to the protection flags based on the section mode
	protection = AdjustProtectionForMode(protection, m_mode);

	// Attempt to (re)commit the pages indicated by the address and length to the specified protection
	NTSTATUS result = NtApi::NtAllocateVirtualMemory(m_process, &address, 0, reinterpret_cast<PSIZE_T>(&length), MEM_COMMIT, protection);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	// Set the corresponding pages bits in the allocation bitmap
	m_allocmap.Set((uintptr_t(address) - uintptr_t(m_address)) / SystemInformation::PageSize, length / SystemInformation::PageSize);

	return address;				// Return the requested base address for convenience
}

//-----------------------------------------------------------------------------
// MemorySection::ChangeMode
//
// Alters the page protection behavior mode for the memory section
//
// Arguments:
//
//	mode			- New mode to set

void MemorySection::ChangeMode(Mode mode)
{
	MEMORY_BASIC_INFORMATION		meminfo;			// Information about a range of pages
	ULONG							previous;			// Previously set protection flags
	NTSTATUS						result;				// Result from function call
	
	if(mode == m_mode) return;							// No changes

	uintptr_t begin = uintptr_t(m_address);				// Starting address
	uintptr_t end = begin + m_length;					// Ending address

	// Use of the allocation bitmap is not necessary here nor would it be of much value since
	// it does not indicate what the protection ranges are.  Use VirtualQueryEx() instead
	while(begin < end) {

		// Get information about the current protection region within the section
		if(!VirtualQueryEx(m_process, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

		// Only attempt to change the protection on regions that have been committed
		if(meminfo.State == MEM_COMMIT) {

			// Determine what the new protection should be for this region of the section
			ULONG newprotection = AdjustProtectionForMode(meminfo.Protect, mode);
			if(newprotection != meminfo.Protect) {

				// Attempt to alter the protection flags of the region for the requested mode
				result = NtApi::NtProtectVirtualMemory(m_process, &meminfo.BaseAddress, &meminfo.RegionSize, newprotection, &previous);
				if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);
			}
		}

		begin += meminfo.RegionSize;							// Move to the next region within the original section
	}

	m_mode = mode;
}

//-----------------------------------------------------------------------------
// MemorySection::Clone (static, private)
//
// Creates a new MemorySection by sharing an existing MemorySection instance
//
// Arguments:
//
//	rhs			- Existing section to be cloned
//	process		- Destination process handle
//	mode		- Clone operation mode

std::unique_ptr<MemorySection> MemorySection::Clone(const std::unique_ptr<MemorySection>& rhs, HANDLE process, Mode mode)
{
	HANDLE						section;						// Duplicated section handle
	void*						mapbase = rhs->m_address;		// Same mapping address
	SIZE_T						maplength = 0;					// Same mapping length
	MEMORY_BASIC_INFORMATION	meminfo;						// Virtual memory region information
	ULONG						previous;						// Previously set protection flags
	NTSTATUS					result;							// Result from function call

	// PRIVATE sections should be generated by Duplicate(), this function clones/shares the section
	_ASSERTE(mode != Mode::Private);

	// Set the new section handle's access mask based on the protection behavior mode.  Shared sections
	// get a handle with full access to the section, copy on write sections get read/execute access to it
	ACCESS_MASK mask = (mode == Mode::CopyOnWrite) ? STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_EXECUTE : SECTION_ALL_ACCESS;

	// Duplicate the section handle with the same attributes as the original and the calculated access mask
	result = NtApi::NtDuplicateObject(NtApi::NtCurrentProcess, rhs->m_section, NtApi::NtCurrentProcess, &section, mask, 0, NtApi::DUPLICATE_SAME_ATTRIBUTES);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	try {

		// Attempt to map the original section into the target process, enabling copy-on-write as necessary
		ULONG prot = (mode == Mode::CopyOnWrite) ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE;
		result = NtApi::NtMapViewOfSection(section, process, &mapbase, 0, 0, nullptr, &maplength, NtApi::ViewUnmap, 0, prot);
		if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

		try {

			uintptr_t begin = uintptr_t(rhs->m_address);
			uintptr_t end = begin + rhs->m_length;

			// Iterate over the source section to apply the same protection flags to the cloned section
			while(begin < end) {

				// Get information about the current region in the source section
				if(!VirtualQueryEx(rhs->m_process, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

				// If this region was committed in the source section, it will have been committed in the new mapping
				if(meminfo.State == MEM_COMMIT) {

					// Adjust and apply the protection flags to the auto-committed region in the cloned mapping
					result = NtApi::NtProtectVirtualMemory(process, &meminfo.BaseAddress, &meminfo.RegionSize, AdjustProtectionForMode(meminfo.Protect, mode), &previous);
					if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);
				}

				begin += meminfo.RegionSize;				// Move to the next region in the source section
			}
		}

		catch(...) { NtApi::NtUnmapViewOfSection(process, mapbase); throw; }
	}

	catch(...) { NtApi::NtClose(section); throw; }

	// Construct the cloned MemorySection instance with a copy of the existing allocation bitmap
	size_t length = maplength;
	return std::make_unique<MemorySection>(process, section, mapbase, length, mode, rhs->m_allocmap);
}

//-----------------------------------------------------------------------------
// MemorySection::Create
//
// Constructs a new anyonymous process virtual memory section
//
// Arguments:
//
//	process			- Target process handle
//	address			- Optional base address for the section
//	length			- Length of the section and mapping
//	mode			- Initial section protection behavior mode
//	flags			- Optional section flags

std::unique_ptr<MemorySection> MemorySection::Create(HANDLE process, void* address, size_t length, Mode mode, uint32_t flags)
{
	HANDLE					section;			// Section handle
	LARGE_INTEGER			sectionlength;		// Section length
	SIZE_T					maplength = 0;		// Mapping length
	NTSTATUS				result;				// Result from function call

	// The only flag currently supported is MEM_TOP_DOWN, which places the mapping
	// at the highest available virtual address in the process
	if(flags & ~(MEM_TOP_DOWN)) throw Win32Exception(ERROR_INVALID_PARAMETER);

	// Align the requested address down to an allocation boundary and adjust length appropriately
	void* mapbase = align::down(address, SystemInformation::AllocationGranularity);
	length = align::up(length + (uintptr_t(address) - uintptr_t(mapbase)), SystemInformation::AllocationGranularity);

	// Copy-on-write sections get read-only access to the section object, otherwise full access is provided
	ACCESS_MASK mask = (mode == Mode::CopyOnWrite) ? STANDARD_RIGHTS_REQUIRED | SECTION_QUERY | SECTION_MAP_READ | SECTION_MAP_EXECUTE : SECTION_ALL_ACCESS;

	// Copy-on-write sections get PAGE_EXECUTE_WRITECOPY by default, otherwise PAGE_EXECUTE_READWRITE
	ULONG protection = (mode == Mode::CopyOnWrite) ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE;

	// Create the section with the calculated access mask and base protection flags
	sectionlength.QuadPart = length;
	result = NtApi::NtCreateSection(&section, mask, nullptr, &sectionlength, protection, SEC_RESERVE, nullptr);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	try {

		// Attempt to map the section into the target process' address space with the specified protection
		result = NtApi::NtMapViewOfSection(section, process, &mapbase, 0, 0, nullptr, &maplength, NtApi::ViewUnmap, flags, protection);
		if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);
	}

	catch(...) { NtApi::NtClose(section); throw; }

	// Construct the MemorySection instance with the new section and mapping attributes
	return std::make_unique<MemorySection>(process, section, mapbase, length, mode);
}

//-----------------------------------------------------------------------------
// MemorySection::Duplicate (private, static)
//
// Creates a new private section in a target process that will contain the same
// data and same protection flags as the source section
//
// Arguments:
//
//	rhs			- Existing section to be duplicated
//	process		- Destination process handle

std::unique_ptr<MemorySection> MemorySection::Duplicate(const std::unique_ptr<MemorySection>& rhs, HANDLE process)
{
	void*						localaddress = nullptr;		// Local in-process mapping to copy from
	SIZE_T						locallength = 0;			// Local mapping length
	MEMORY_BASIC_INFORMATION	meminfo;					// Virtual memory region information
	NTSTATUS					result;						// Result from function call

	// Create a new section reservation with the same address and length as the original
	std::unique_ptr<MemorySection> duplicate = Create(process, rhs->m_address, rhs->m_length, Mode::Private);

	// Map the section into the current process as READONLY so that the data can be copied into the new section
	result = NtApi::NtMapViewOfSection(rhs->m_section, NtApi::NtCurrentProcess, &localaddress, 0, 0, nullptr, &locallength, NtApi::ViewUnmap, 0, PAGE_READONLY);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	try {

		uintptr_t begin = uintptr_t(rhs->m_address);
		uintptr_t end = begin + rhs->m_length;

		// Iterate over the source section to apply the same status and protection flags to the new section
		while(begin < end) {

			// Get information about the current region within the source section
			if(!VirtualQueryEx(rhs->m_process, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

			// If the region is committed, copy the data from the source region to the destination region
			if(meminfo.State == MEM_COMMIT) {

				// Allocate/commit the duplicate region with READWRITE access in order to write into it
				duplicate->Allocate(meminfo.BaseAddress, meminfo.RegionSize, PAGE_READWRITE);

				// Use NtWriteVirtualMemory to copy the region from the local mapping into the duplicate mapping
				void* sourceaddress = reinterpret_cast<void*>(uintptr_t(localaddress) + (uintptr_t(meminfo.BaseAddress) - uintptr_t(rhs->m_address)));
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
// MemorySection::FromSection (static)
//
// Creates a new MemorySection in another process

std::unique_ptr<MemorySection> MemorySection::FromSection(const std::unique_ptr<MemorySection>& rhs, HANDLE process, Mode mode)
{
	// PRIVATE: Duplicate the section in the target process
	if(mode == Mode::Private) return Duplicate(rhs, process);

	// SHARED/COPY-ON-WRITE: Clone the section in the target process
	else return Clone(rhs, process, mode);
}

//-----------------------------------------------------------------------------
// MemorySection::Protect
//
// Applies new protection flags to pages within the memory section
//
// Arguments:
//
//	address			- Base address to begin applying new protection
//	length			- Length of the region to be protected
//	protection		- New protection flags for the memory section

void MemorySection::Protect(void* address, size_t length, uint32_t protection)
{
	ULONG				previous;					// Previously set protection flags

	// Verify that the requested range falls within this section's virtual address space
	if(address < m_address) throw Win32Exception(ERROR_INVALID_ADDRESS);
	if(uintptr_t(address) + length > uintptr_t(m_address) + m_length) throw Win32Exception(ERROR_INVALID_ADDRESS);

	// Verify that the entire range is allocated/committed by checking the allocation bitmap
	size_t startbit = align::down(uintptr_t(address) - uintptr_t(m_address), SystemInformation::PageSize) / SystemInformation::PageSize;
	size_t endbit = (align::up(uintptr_t(address) + length, SystemInformation::PageSize) - uintptr_t(m_address)) / SystemInformation::PageSize;
	if(!m_allocmap.AreBitsSet(startbit, endbit - startbit)) throw Win32Exception(ERROR_INVALID_ADDRESS);

	// Make any necessary changes to the protection flags based on the section mode
	protection = AdjustProtectionForMode(protection, m_mode);

	// Attempt to change the protection flags for the specified range of memory pages
	NTSTATUS result = NtApi::NtProtectVirtualMemory(m_process, &address, reinterpret_cast<PSIZE_T>(&length), protection, &previous);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// MemorySection::Release
//
// Releases (resets) pages within the memory section
//
// Arguments:
//
//	address			- Base address of the pages to be committed
//	length			- Length of the region to be committed

void MemorySection::Release(void* address, size_t length)
{
	ULONG				previous;					// Previously set page protection flags

	// Verify that the requested range falls within this section's virtual address space
	if(address < m_address) throw Win32Exception(ERROR_INVALID_ADDRESS);
	if(uintptr_t(address) + length > uintptr_t(m_address) + m_length) throw Win32Exception(ERROR_INVALID_ADDRESS);

	// NOTE: Do not check the bitmap for allocation here; it will not be considered an error to release
	// memory that is not allocated.

	// Attempt to change the protection of the pages involved to PAGE_NOACCESS since they can't be decommitted
	NTSTATUS result = NtApi::NtProtectVirtualMemory(m_process, &address, reinterpret_cast<PSIZE_T>(&length), PAGE_NOACCESS, &previous);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	// Attempt to reset the pages indicated by the address and length.  Note that MEM_RESET rounds the base address
	// UP within the region rather than DOWN like a normal operation would, therefore length will be reduced rather than increased
	result = NtApi::NtAllocateVirtualMemory(m_process, &address, 0, reinterpret_cast<PSIZE_T>(&length), MEM_RESET, PAGE_NOACCESS);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	// Unlock the pages from physical memory (this operation will typically fail, don't bother checking the result)
	NtApi::NtUnlockVirtualMemory(m_process, &address, reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);

	// Clear the corresponding page bits in the allocation bitmap
	m_allocmap.Clear((uintptr_t(address) - uintptr_t(m_address)) / SystemInformation::PageSize, length / SystemInformation::PageSize);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
