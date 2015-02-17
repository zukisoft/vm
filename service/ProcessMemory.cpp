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
#include "ProcessMemory.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------

const void* ProcessMemory::Allocate(size_t length, int prot)
{
	return Allocate(nullptr, length, prot);
}

//------------------------------------------------------------------------------
// ProcessMemory::Allocate
//
// Allocates virtual memory
//
// Arguments:
//
//	address			- Optional base address or nullptr
//	length			- Required allocation length
//	prot			- Linux memory protection flags for the new region

const void* ProcessMemory::Allocate(const void* address, size_t length, int prot)
{
	MEMORY_BASIC_INFORMATION					meminfo;		// Virtual memory information

	// Allocations cannot be zero-length
	if(length == 0) throw Win32Exception(ERROR_INVALID_PARAMETER);

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock writer(m_sectionlock);

	// No specific address was requested, let the operating system decide where it should go
	if(address == nullptr) {

		std::unique_ptr<MemorySection> section = MemorySection::Create(m_nativehandle, align::up(length, SystemInformation::AllocationGranularity));
		address = section->Allocate(section->BaseAddress, length, uapi::LinuxProtToWindowsPageFlags(prot));
		m_sections.push_back(std::move(section));
		return address;
	}

	// A specific address was requested, first scan over the virtual address space and fill in any holes
	// with new meory sections to ensure a contiguous region
	uintptr_t fillbegin = align::down(uintptr_t(address), SystemInformation::AllocationGranularity);
	uintptr_t fillend = align::up((uintptr_t(address) + length), SystemInformation::AllocationGranularity);

	while(fillbegin < fillend) {

		// Query the information about the virtual memory beginning at the current address
		if(!VirtualQueryEx(m_nativehandle, reinterpret_cast<void*>(fillbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

		// If the region is free (MEM_FREE), create a new memory section in the free space
		if(meminfo.State == MEM_FREE) {

			size_t filllength = min(meminfo.RegionSize, align::up(fillend - fillbegin, SystemInformation::AllocationGranularity));
			m_sections.emplace_back(MemorySection::Create(m_nativehandle, meminfo.BaseAddress, filllength));
		}

		fillbegin += meminfo.RegionSize;
	}

	// The entire required virtual address space is now available for the allocation operation
	uintptr_t allocbegin = uintptr_t(address);
	uintptr_t allocend = allocbegin + length;

	while(allocbegin < allocend) {

		// Locate the section object that matches the current allocation base address
		const auto& found = std::find_if(m_sections.begin(), m_sections.end(), [&](const std::unique_ptr<MemorySection>& section) -> bool {
			return ((allocbegin >= uintptr_t(section->BaseAddress)) && (allocbegin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, throw ERROR_INVALID_ADDRESS
		if(found == m_sections.end()) throw Win32Exception(ERROR_INVALID_ADDRESS);

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		const auto& section = *found;

		// Determine the length of the allocation to request from this section and request it
		size_t alloclen = min(section->Length - (allocbegin - uintptr_t(section->BaseAddress)), allocend - allocbegin);
		section->Allocate(reinterpret_cast<void*>(allocbegin), alloclen, uapi::LinuxProtToWindowsPageFlags(prot));

		allocbegin += alloclen;
	}

	return address;					// Return the originally requested address
}

//-----------------------------------------------------------------------------
// ProcessMemory::Clear
//
// Removes all allocated virtual memory from the native process
//
// Arguments:
//
//	NONE

void ProcessMemory::Clear(void)
{
	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock writer(m_sectionlock);

	// Clearing the vector<> will release all of the section instances
	m_sections.clear();
}

//-----------------------------------------------------------------------------
// ProcessMemory::Clone (static)
//
// Clones the virtual memory from an existing instance as copy-on-write
//
// Arguments:
//
//	existing		- Reference to an existing ProcessMemory instance
//	nativeprocess	- Target native process handle

std::shared_ptr<ProcessMemory> ProcessMemory::Clone(const std::shared_ptr<ProcessMemory>& existing, HANDLE nativeprocess)
{
	section_vector_t		newsections;			// New copy-on-write section collection

	// Prevent changes to the existing process memory layout
	section_lock_t::scoped_lock_read reader(existing->m_sectionlock);

	// Iterate over the existing memory sections
	for(auto iterator = existing->m_sections.begin(); iterator != existing->m_sections.end(); iterator++) {

		// Clone the existing memory section as copy-on-write from the source
		newsections.push_back(MemorySection::FromSection(*iterator, nativeprocess, MemorySection::Mode::CopyOnWrite));

		// Ensure that the source section mode is also changed to copy-on-write
		(*iterator)->ChangeMode(MemorySection::Mode::CopyOnWrite);
	}

	// Construct a new ProcessMemory instance, moving the new section collection into it
	return std::make_shared<ProcessMemory>(nativeprocess, std::move(newsections));
}

//-----------------------------------------------------------------------------
// ProcessMemory::Create (static)
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	nativeprocess	- Target native process handle

std::shared_ptr<ProcessMemory> ProcessMemory::Create(HANDLE nativeprocess)
{
	// Use an empty collection to initialize the ProcessMemory instance
	return std::make_shared<ProcessMemory>(nativeprocess, section_vector_t());
}

//-----------------------------------------------------------------------------
// ProcessMemory::Duplicate (static)
//
// Duplicates the virtual memory from an existing instance
//
// Arguments:
//
//	existing		- Reference to an existing ProcessMemory instance
//	nativeprocess	- Target native process handle

std::shared_ptr<ProcessMemory> ProcessMemory::Duplicate(const std::shared_ptr<ProcessMemory>& existing, HANDLE nativeprocess)
{
	section_vector_t		newsections;			// New copy-on-write section collection

	// Prevent changes to the existing process memory layout
	section_lock_t::scoped_lock_read reader(existing->m_sectionlock);

	// Iterate over the existing memory sections
	for(auto iterator = existing->m_sections.begin(); iterator != existing->m_sections.end(); iterator++) {

		// Duplicate the existing memory section into a new private section
		newsections.push_back(MemorySection::FromSection(*iterator, nativeprocess, MemorySection::Mode::Private));
	}

	// Construct a new ProcessMemory instance, moving the new section collection into it
	return std::make_shared<ProcessMemory>(nativeprocess, std::move(newsections));
}

//-----------------------------------------------------------------------------
// ProcessMemory::Protect
//
// Assigns memory protection flags for an allocated region of memory
//
// Arguments:
//
//	address		- Base address of the region to be protected
//	length		- Length of the region to be protected
//	prot		- Linux memory protection flags for the region

void ProcessMemory::Protect(const void* address, size_t length, int prot)
{
	// Determine the starting and ending points for the operation
	uintptr_t begin = uintptr_t(address);
	uintptr_t end = begin + length;

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock_read reader(m_sectionlock);

	while(begin < end) {

		// Locate the section object that matches the current base address
		const auto& found = std::find_if(m_sections.begin(), m_sections.end(), [&](const std::unique_ptr<MemorySection>& section) -> bool {
			return ((begin >= uintptr_t(section->BaseAddress)) && (begin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, throw ERROR_INVALID_ADDRESS
		if(found == m_sections.end()) throw Win32Exception(ERROR_INVALID_ADDRESS);

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		const auto& section = *found;

		// Determine the length of the allocation to request from this section and request it
		size_t protectlen = min(section->Length - (begin - uintptr_t(section->BaseAddress)), end - begin);
		section->Protect(reinterpret_cast<void*>(begin), protectlen, uapi::LinuxProtToWindowsPageFlags(prot));

		begin += protectlen;
	}
}

//-----------------------------------------------------------------------------
// ProcessMemory::Read
//
// Reads data from the process address space
//
// Arguments:
//
//	address		- Address in the process from which to read
//	buffer		- Output data buffer
//	length		- Size of the output data buffer

size_t ProcessMemory::Read(const void* address, void* buffer, size_t length)
{
	SIZE_T						read;			// Number of bytes read

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock_read reader(m_sectionlock);

	// Attempt to read the requested data from the native process
	NTSTATUS result = NtApi::NtReadVirtualMemory(m_nativehandle, address, buffer, length, &read);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException(LINUX_EFAULT, StructuredException(result));

	return static_cast<size_t>(read);
}

//-----------------------------------------------------------------------------
// ProcessMemory::Release
//
// Releases memory from the process address space
//
// Arguments:
//
//	address		- Base address of the memory region to be released
//	length		- Number of bytes to be released

void ProcessMemory::Release(const void* address, size_t length)
{
	uintptr_t begin = uintptr_t(address);			// Start address as uintptr_t
	uintptr_t end = begin + length;					// End address as uintptr_t

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock writer(m_sectionlock);

	while(begin < end) {

		// Locate the section object that matches the specified base address
		const auto& found = std::find_if(m_sections.begin(), m_sections.end(), [&](const std::unique_ptr<MemorySection>& section) -> bool {
			return ((begin >= uintptr_t(section->BaseAddress)) && (begin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, treat this as a no-op -- don't throw an exception
		if(found == m_sections.end()) return;

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		const auto& section = *found;

		// Determine how much to release from this section and release it
		size_t freelength = min(section->Length - (begin - uintptr_t(section->BaseAddress)), end - begin);
		section->Release(reinterpret_cast<void*>(begin), freelength);

		// If the section is empty after the release, remove it from the process
		if(section->Empty) m_sections.erase(found);

		begin += freelength;
	}
}

//-----------------------------------------------------------------------------
// ProcessMemory::Write
//
// Writes data into the process address space
//
// Arguments:
//
//	address		- Address in the client process from which to read
//	buffer		- Output data buffer
//	length		- Size of the output data buffer

size_t ProcessMemory::Write(const void* address, const void* buffer, size_t length)
{
	SIZE_T					written;			// Number of bytes written

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock_read reader(m_sectionlock);

	// Attempt to write the requested data into the native process
	NTSTATUS result = NtApi::NtWriteVirtualMemory(m_nativehandle, address, buffer, length, &written);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException(LINUX_EFAULT, StructuredException(result));

	return written;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
