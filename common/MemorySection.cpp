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
#include "MemorySection.h"

#pragma warning(push, 4)

// MemorySection::NtAllocateVirtualMemory
//
// Reserves, commits, or both, a region of pages within the user-mode virtual address space of a specified process
MemorySection::NtAllocateVirtualMemoryFunc MemorySection::NtAllocateVirtualMemory = 
reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PVOID, ULONG_PTR, PSIZE_T, ULONG, ULONG)>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtAllocateVirtualMemory"); 
}());

// MemorySection::NtClose
//
// Closes an object handle
MemorySection::NtCloseFunc MemorySection::NtClose =
reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE)>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtClose"); 
}());

// MemorySection::NtCreateSection
//
// Creates a section object
MemorySection::NtCreateSectionFunc MemorySection::NtCreateSection =
reinterpret_cast<NTSTATUS(NTAPI*)(PHANDLE, ACCESS_MASK, POBJECT_ATTRIBUTES, PLARGE_INTEGER, ULONG, ULONG, HANDLE)>([]() -> FARPROC {
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtCreateSection"); 
}());

// MemorySection::NtCurrentProcess
//
// Pseudo-handle representing the current process
HANDLE MemorySection::NtCurrentProcess = reinterpret_cast<HANDLE>(static_cast<LONG_PTR>(-1));

// MemorySection::NtDuplicateObject
//
// Creates a handle that is a duplicate of the specified source handle
MemorySection::NtDuplicateObjectFunc MemorySection::NtDuplicateObject =
reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, HANDLE, HANDLE, PHANDLE, ACCESS_MASK, ULONG, ULONG)>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtDuplicateObject"); 
}());

// MemorySection::NtFreeVirtualMemory
//
// Releases, decommits, or both, a region of pages within the virtual address space of a specified process
MemorySection::NtFreeVirtualMemoryFunc MemorySection::NtFreeVirtualMemory =
reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PVOID, PSIZE_T, ULONG)>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtFreeVirtualMemory"); 
}());

// MemorySection::NtMapViewOfSection
//
// Maps a view of a section into the virtual address space of a subject process
MemorySection::NtMapViewOfSectionFunc MemorySection::NtMapViewOfSection =
reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, HANDLE, PVOID, ULONG_PTR, SIZE_T, PLARGE_INTEGER, PSIZE_T, SECTION_INHERIT, ULONG, ULONG)>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtMapViewOfSection"); 
}());

// MemorySection::NtProtectVirtualMemory
//
// Changes the protection on a region of committed pages in the virtual address space of a subject process
MemorySection::NtProtectVirtualMemoryFunc MemorySection::NtProtectVirtualMemory =
reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PVOID*, PSIZE_T, ULONG, PULONG)>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtProtectVirtualMemory");
}());

// MemorySection::NtUnmapViewOfSection
//
// Unmaps a view of a section from the virtual address space of a subject process
MemorySection::NtUnmapViewOfSectionFunc MemorySection::NtUnmapViewOfSection =
reinterpret_cast<NTSTATUS(NTAPI*)(HANDLE, PVOID)>([]() -> FARPROC { 
	return GetProcAddress(LoadLibrary(_T("ntdll.dll")), "NtUnmapViewOfSection"); 
}());

//-----------------------------------------------------------------------------
// MemorySection Destructor

MemorySection::~MemorySection()
{
	// Unmap the section view from the process and close the section handle
	NtUnmapViewOfSection(m_process, m_address);
	NtClose(m_section);
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
	void*						address = m_address;	// Same mapping address
	size_t						length = m_length;		// Same mapping length
	MEMORY_BASIC_INFORMATION	meminfo;				// Virtual memory region information
	NTSTATUS					result;					// Result from function call

	// Duplication of the section is best served in its own function
	if(mode == CloneMode::Duplicate) return Duplicate(process);

	// Set the new section handle's access mask based on the cloning mode.  If copy-on-write access
	// is requested, allow only READ and EXECUTE access to the original section
	ACCESS_MASK mask = (mode == CloneMode::SharedCopyOnWrite) ? SECTION_MAP_READ | SECTION_MAP_EXECUTE : SECTION_ALL_ACCESS;

	// Duplicate the section handle with the same attributes as the original and the calculated access mask
	result = NtDuplicateObject(NtCurrentProcess, m_section, NtCurrentProcess, &section, mask, 0, DUPLICATE_SAME_ATTRIBUTES);
	if(result != STATUS_SUCCESS) throw StructuredException(result);

	try {

		// Attempt to map the original section into the target process, enabling copy-on-write as necessary
		ULONG prot = (mode == CloneMode::SharedCopyOnWrite) ? PAGE_EXECUTE_WRITECOPY : PAGE_EXECUTE_READWRITE;
		result = NtMapViewOfSection(section, process, &address, 0, 0, nullptr, reinterpret_cast<PSIZE_T>(&length), ViewUnmap, 0, prot);
		if(result != STATUS_SUCCESS) throw StructuredException(result);

		try {

			uintptr_t begin = uintptr_t(m_address);
			uintptr_t end = begin + m_length;

			// Iterate over the source section to apply the same status and protection flags to the cloned section
			while(begin < end) {

				// Get information about the current region in the source section
				if(!VirtualQueryEx(m_process, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

				// If this region was committed in the source section, also commit it in the cloned section
				if(meminfo.State == MEM_COMMIT) {

					// If the cloning mode is copy-on-write, READWRITE protections must be adjusted to WRITECOPY protections
					if(mode == CloneMode::SharedCopyOnWrite) {

						if(meminfo.Protect == PAGE_READWRITE) meminfo.Protect = PAGE_WRITECOPY;
						else if(meminfo.Protect == PAGE_EXECUTE_READWRITE) meminfo.Protect = PAGE_EXECUTE_WRITECOPY;
					}

					// Commit the region in the cloned section with the proper protection flags
					NTSTATUS result = NtAllocateVirtualMemory(process, &meminfo.BaseAddress, 0, &meminfo.RegionSize, MEM_COMMIT, meminfo.Protect);
					if(result != STATUS_SUCCESS) throw StructuredException(result);
				}

				begin += meminfo.RegionSize;				// Move to the next region in the source section
			}
		}

		catch(...) { NtUnmapViewOfSection(process, address); throw; }
	}

	catch(...) { NtClose(section); throw; }

	// Construct the cloned MemorySection instance with the new handle, address and length
	return std::make_unique<MemorySection>(process, section, address, length);
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
	// The system will automatically align the provided address and length to page boundaries
	NTSTATUS result = NtAllocateVirtualMemory(m_process, &address, 0, reinterpret_cast<PSIZE_T>(&length), MEM_COMMIT, protect);
	if(result != STATUS_SUCCESS) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// MemorySection::Decommit
//
// Decommits page(s) of memory from within the section 
//
// Arguments:
//
//	address		- Base address to be decommitted
//	length		- Length of the region to be decommitted

void MemorySection::Decommit(void* address, size_t length)
{
	// The system will automatically align the provided address and length to page boundaries
	NTSTATUS result = NtFreeVirtualMemory(m_process, address, reinterpret_cast<PSIZE_T>(&length), MEM_DECOMMIT);
	if(result != STATUS_SUCCESS) throw StructuredException(result);
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
	SIZE_T						written;					// Bytes written during a copy operation
	NTSTATUS					result;						// Result from function call

	// Create a new section reservation with the same address and length as the original
	std::unique_ptr<MemorySection> duplicate = Reserve(process, m_address, m_length);

	// Map the section into the current process as READONLY so that the data can be copied into the new section
	result = NtMapViewOfSection(m_section, NtCurrentProcess, &localaddress, 0, 0, nullptr, &locallength, ViewUnmap, 0, PAGE_READONLY);
	if(result != STATUS_SUCCESS) throw StructuredException(result);

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

				// Use WriteProcessMemory to copy the region from the local mapping
				void* sourceaddress = reinterpret_cast<void*>(uintptr_t(localaddress) + (uintptr_t(meminfo.BaseAddress) - uintptr_t(m_address)));
				if(!WriteProcessMemory(process, meminfo.BaseAddress, sourceaddress, meminfo.RegionSize, &written)) throw Win32Exception();
				_ASSERTE(written == meminfo.RegionSize);

				// Apply the source region protection flags to the destination region
				duplicate->Protect(meminfo.BaseAddress, meminfo.RegionSize, meminfo.Protect);
			}

			begin += meminfo.RegionSize;							// Move to the next region within the original section
		}

		NtUnmapViewOfSection(NtCurrentProcess, localaddress);		// Remove the local process mapping
	}

	catch(...) { NtUnmapViewOfSection(NtCurrentProcess, localaddress); throw; }

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
	ULONG				previous;				// Previously set protection flags

	// The system will automatically align the provided address and length to page boundaries
	NTSTATUS result = NtProtectVirtualMemory(m_process, &address, reinterpret_cast<PSIZE_T>(&length), protect, &previous);
	if(result != STATUS_SUCCESS) throw StructuredException(result);

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
//	mapflags		- Mapping operation flags (limited to MEM_TOP_DOWN and MEM_LARGE_PAGES)

std::unique_ptr<MemorySection> MemorySection::Reserve(HANDLE process, void* address, size_t length, uint32_t mapflags)
{
	HANDLE					section;			// Section handle
	LARGE_INTEGER			sectionlength;		// Section length
	NTSTATUS				result;				// Result from function call

	// Verify that no invalid flags have been specified
	if(mapflags & ~(MEM_TOP_DOWN | MEM_LARGE_PAGES)) throw Win32Exception(ERROR_INVALID_PARAMETER);

	// Align the requested address down to an allocation boundary and adjust length appropriately
	void* aligned = align::down(address, SystemInformation::AllocationGranularity);
	length += (uintptr_t(address) - uintptr_t(aligned));

	// Allocate the section with PAGE_EXECUTE_READWRITE to allow the same protection when the section is mapped
	sectionlength.QuadPart = length;
	result = NtCreateSection(&section, SECTION_ALL_ACCESS, nullptr, &sectionlength, PAGE_EXECUTE_READWRITE, SEC_RESERVE, nullptr);
	if(result != STATUS_SUCCESS) throw StructuredException(result);

	try {

		// Attempt to map the section into the target process' address space with PAGE_EXECUTE_READWRITE, which will allow
		// any valid protection to be specified when pages in the region are committed
		result = NtMapViewOfSection(section, process, &aligned, 0, 0, nullptr, reinterpret_cast<PSIZE_T>(&length), ViewUnmap, mapflags, PAGE_EXECUTE_READWRITE);
		if(result != STATUS_SUCCESS) throw StructuredException(result);
	}

	catch(...) { NtClose(section); throw; }

	// Construct the MemorySection instance with the new section attributes
	return std::make_unique<MemorySection>(process, section, aligned, length);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
