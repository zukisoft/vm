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
// ProcessMemory Constructor
//
// Arguments:
//
//	process		- NativeHandle for the operating system process
//	section		- Collection of MemorySections to initialize the instance

ProcessMemory::ProcessMemory(const std::shared_ptr<NativeHandle>& process, section_vector_t&& sections)
	: m_process(process), m_sections(std::move(sections)) {}

//------------------------------------------------------------------------------
// ProcessMemory::Allocate
//
// Allocates virtual memory
//
// Arguments:
//
//	length			- Required allocation length
//	prot			- Linux memory protection flags for the new region

const void* ProcessMemory::Allocate(size_t length, int prot)
{
	// Let the native operating system decide where to allocate the section
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
	if(length == 0) throw LinuxException(LINUX_EINVAL);

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock writer(m_sectionlock);

	// No specific address was requested, let the operating system decide where it should go
	if(address == nullptr) {

		std::unique_ptr<MemorySection> section = MemorySection::Create(m_process->Handle, align::up(length, SystemInformation::AllocationGranularity));
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
		if(!VirtualQueryEx(m_process->Handle, reinterpret_cast<void*>(fillbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) 
			throw LinuxException(LINUX_EACCES, Win32Exception());

		// If the region is free (MEM_FREE), create a new memory section in the free space
		if(meminfo.State == MEM_FREE) {

			size_t filllength = min(meminfo.RegionSize, align::up(fillend - fillbegin, SystemInformation::AllocationGranularity));
			m_sections.emplace_back(MemorySection::Create(m_process->Handle, meminfo.BaseAddress, filllength));
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

		// No matching section object exists, throw EINVAL/ERROR_INVALID_ADDRESS
		if(found == m_sections.end()) throw LinuxException(LINUX_EINVAL, Win32Exception(ERROR_INVALID_ADDRESS));

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
// ProcessMemory::Create (static)
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	process		- Target native process handle

std::unique_ptr<ProcessMemory> ProcessMemory::Create(const std::shared_ptr<NativeHandle>& process)
{
	// Use an empty collection to initialize the ProcessMemory instance
	return std::make_unique<ProcessMemory>(process, section_vector_t());
}

//-----------------------------------------------------------------------------
// ProcessMemory::Duplicate (static)
//
// Duplicates the address space from an existing ProcessMemory instance
//
// Arguments:
//
//	process			- Target native process handle
//	existing		- Reference to an existing ProcessMemory instance
//	mode			- Address space duplication mode

std::unique_ptr<ProcessMemory> ProcessMemory::Duplicate(const std::shared_ptr<NativeHandle>& process, const std::unique_ptr<ProcessMemory>& existing, 
	DuplicationMode mode)
{
	section_vector_t		newsections;			// New copy-on-write section collection

	// Prevent changes to the existing process memory layout
	section_lock_t::scoped_lock_read reader(existing->m_sectionlock);

	// Iterate over the existing memory sections
	for(auto iterator = existing->m_sections.begin(); iterator != existing->m_sections.end(); iterator++) {

		// Duplicate the existing memory section into a new memory section for the target process
		newsections.push_back(MemorySection::FromSection(*iterator, process->Handle, static_cast<MemorySection::Mode>(mode)));

		//
		// TODO: REMOVE THIS WITH A BETTER PLAN -- NEED TO CHOOSE BASED ON THE SECTION TYPE FOR SHARED MMAPS
		// but ... if you don't change the parent mode appropriately, bad things will happen
		//

		(*iterator)->ChangeMode(MemorySection::Mode::CopyOnWrite);
	}

	// Construct a new ProcessMemory instance, moving the new section collection into it
	return std::make_unique<ProcessMemory>(process, std::move(newsections));
}

//-----------------------------------------------------------------------------
// ProcessMemory::Guard
//
// Creates guard pages within an allocated region of memory
//
// Arguments:
//
//	address		- Base address of the guard page region
//	length		- Length of the guard page region
//	prot		- Linux memory protection flags for the region

void ProcessMemory::Guard(const void* address, size_t length, int prot)
{
	// Use the common internal version that accepts windows page flags
	return ProtectInternal(address, length, uapi::LinuxProtToWindowsPageFlags(prot) | PAGE_GUARD);
}
	
//-----------------------------------------------------------------------------
// ProcessMemory::Lock
//
// Attempts to lock a region of data into the process working set, does not throw 
// an exception if it fails
//
// Arguments:
//
//	address		- Base address of the region to be locked
//	length		- Length of the region to be locked

void ProcessMemory::Lock(const void* address, size_t length) const
{
	// todo: I want to see if this ever works, assert to remind myself
	_ASSERTE(false);

	// Attempt to lock the requested region into the process working set
	LPVOID addr = const_cast<void*>(address);
	NtApi::NtLockVirtualMemory(m_process->Handle, &addr, reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);
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
	// Use the common internal version that accepts windows page flags
	return ProtectInternal(address, length, uapi::LinuxProtToWindowsPageFlags(prot));
}

//-----------------------------------------------------------------------------
// ProcessMemory::ProtectInternal (private)
//
// Assigns memory protection flags for an allocated region of memory
//
// Arguments:
//
//	address		- Base address of the region to be protected
//	length		- Length of the region to be protected
//	winprot		- Windows memory protection flags for the region

void ProcessMemory::ProtectInternal(const void* address, size_t length, uint32_t winprot)
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

		// No matching section object exists, throw EINVAL/ERROR_INVALID_ADDRESS
		if(found == m_sections.end()) throw LinuxException(LINUX_EINVAL, Win32Exception(ERROR_INVALID_ADDRESS));

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		const auto& section = *found;

		// Determine the length of the allocation to request from this section and request it
		size_t protectlen = min(section->Length - (begin - uintptr_t(section->BaseAddress)), end - begin);
		section->Protect(reinterpret_cast<void*>(begin), protectlen, winprot);

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
	NTSTATUS result = NtApi::NtReadVirtualMemory(m_process->Handle, address, buffer, length, &read);
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

		// No matching section object exists, treat this as a no-op
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
// ProcessMemory::Unlock
//
// Attempts to unlock a region of data from the process working set, does not 
// throw an exception if it fails
//
// Arguments:
//
//	address		- Base address of the region to be unlocked
//	length		- Length of the region to be unlocked

void ProcessMemory::Unlock(const void* address, size_t length) const
{
	// Attempt to unlock the requested region from the process working set
	LPVOID addr = const_cast<void*>(address);
	NtApi::NtUnlockVirtualMemory(m_process->Handle, &addr, reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);
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
	NTSTATUS result = NtApi::NtWriteVirtualMemory(m_process->Handle, address, buffer, length, &written);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException(LINUX_EFAULT, StructuredException(result));

	return written;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
