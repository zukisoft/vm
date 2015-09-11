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
#include "Host.h"

#include "MemorySection.h"
#include "LinuxException.h"
#include "NativeProcess.h"
#include "NtApi.h"
#include "StructuredException.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

//
// HOST::MEMORYPROTECTION
//

// Host::MemoryProtection::Atomic (static)
//
const Host::MemoryProtection Host::MemoryProtection::Atomic{ LINUX_PROT_SEM };

// Host::MemoryProtection::Execute (static)
//
const Host::MemoryProtection Host::MemoryProtection::Execute{ LINUX_PROT_EXEC };

// Host::MemoryProtection::None (static)
//
const Host::MemoryProtection Host::MemoryProtection::None{ LINUX_PROT_NONE };

// Host::MemoryProtection::Read (static)
//
const Host::MemoryProtection Host::MemoryProtection::Read{ LINUX_PROT_READ };

// Host::MemoryProtection::Write (static)
//
const Host::MemoryProtection Host::MemoryProtection::Write{ LINUX_PROT_WRITE };

//-----------------------------------------------------------------------------
// Conversions
//-----------------------------------------------------------------------------

// Host::MemoryProtection --> DWORD
//
// Converts MemoryProtection bitmask into the closest equivalent Win32 API bitmask
template<> DWORD convert<DWORD>(const Host::MemoryProtection& rhs)
{
	using prot = Host::MemoryProtection;

	prot flags{ rhs & (prot::Execute | prot::Write | prot::Read) };

	if(flags == prot::Execute)									return PAGE_EXECUTE;
	else if(flags == prot::Read)								return PAGE_READONLY;
	else if(flags == prot::Write)								return PAGE_READWRITE;
	else if(flags == prot::Execute | prot::Read)				return PAGE_EXECUTE_READ;
	else if(flags == prot::Execute | prot::Write)				return PAGE_EXECUTE_READWRITE;
	else if(flags == prot::Read | prot::Write)					return PAGE_READWRITE;
	else if(flags == prot::Execute | prot::Read | prot::Write)	return PAGE_EXECUTE_READWRITE;

	return PAGE_NOACCESS;
}

//-----------------------------------------------------------------------------
// Host Constructor
//
// Arguments:
//
//	nativeproc		- NativeProcess instance to take ownership of
//	sections		- Memory sections collection for the host process

Host::Host(nativeproc_t nativeproc, section_vector_t&& sections) : m_nativeproc(std::move(nativeproc)), m_sections(std::move(sections))
{
}

//-----------------------------------------------------------------------------
// Host Destructor

Host::~Host()
{
}

//------------------------------------------------------------------------------
// Host::AllocateMemory
//
// Allocates virtual memory
//
// Arguments:
//
//	length			- Required allocation length
//	prot			- Memory protection flags for the new region

const void* Host::AllocateMemory(size_t length, MemoryProtection prot)
{
	// Let the native operating system decide where to allocate the section
	return AllocateMemory(nullptr, length, prot);
}

//------------------------------------------------------------------------------
// Host::AllocateMemory
//
// Allocates virtual memory
//
// Arguments:
//
//	address			- Optional base address or nullptr
//	length			- Required allocation length
//	prot			- Memory protection flags for the new region

const void* Host::AllocateMemory(const void* address, size_t length, MemoryProtection prot)
{
	MEMORY_BASIC_INFORMATION					meminfo;		// Virtual memory information

	// Allocations cannot be zero-length
	if(length == 0) throw LinuxException{ LINUX_EINVAL };

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	// No specific address was requested, let the operating system decide where it should go
	if(address == nullptr) {

		auto section = MemorySection::Create(m_nativeproc->ProcessHandle, align::up(length, SystemInformation::AllocationGranularity));
		address = section->Allocate(section->BaseAddress, length, convert<DWORD>(prot));
		m_sections.push_back(std::move(section));
		return address;
	}

	// A specific address was requested, first scan over the virtual address space and fill in any holes
	// with new meory sections to ensure a contiguous region
	uintptr_t fillbegin = align::down(uintptr_t(address), SystemInformation::AllocationGranularity);
	uintptr_t fillend = align::up((uintptr_t(address) + length), SystemInformation::AllocationGranularity);

	while(fillbegin < fillend) {

		// Query the information about the virtual memory beginning at the current address
		if(!VirtualQueryEx(m_nativeproc->ProcessHandle, reinterpret_cast<void*>(fillbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) 
			throw LinuxException{ LINUX_EACCES, Win32Exception{} };

		// If the region is free (MEM_FREE), create a new memory section in the free space
		if(meminfo.State == MEM_FREE) {

			size_t filllength = std::min(static_cast<size_t>(meminfo.RegionSize), align::up(fillend - fillbegin, SystemInformation::AllocationGranularity));
			m_sections.emplace_back(MemorySection::Create(m_nativeproc->ProcessHandle, meminfo.BaseAddress, filllength));
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
		if(found == m_sections.end()) throw LinuxException{ LINUX_EINVAL, Win32Exception{ ERROR_INVALID_ADDRESS } };

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		const auto& section = *found;

		// Determine the length of the allocation to request from this section and request it
		size_t alloclen = std::min(section->Length - (allocbegin - uintptr_t(section->BaseAddress)), allocend - allocbegin);
		section->Allocate(reinterpret_cast<void*>(allocbegin), alloclen, convert<DWORD>(prot));

		allocbegin += alloclen;
	}

	return address;					// Return the originally requested address
}

//------------------------------------------------------------------------------
// Host::getArchitecture
//
// Gets the architecture of the native hosting process

enum class Architecture Host::getArchitecture(void) const
{
	return m_nativeproc->Architecture;
}

//-----------------------------------------------------------------------------
// Host::ClearMemory
//
// Removes all allocated virtual memory from the native process
//
// Arguments:
//
//	NONE

void Host::ClearMemory(void)
{
	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	// Clearing the vector<> will release all of the section instances
	m_sections.clear();
}

//-----------------------------------------------------------------------------
// Host::Create (static)
//
// Creates a new Host instance from an existing NativeProcess instance
//
// Arguments:
//
//	nativeproc		- NativeProcess instance to take ownership of

std::unique_ptr<Host> Host::Create(std::unique_ptr<NativeProcess> nativeproc)
{
	// Create the host with an initially empty memory sections collection
	return std::make_unique<Host>(std::move(nativeproc), section_vector_t());
}

//-----------------------------------------------------------------------------
// Host::GuardMemory
//
// Creates guard pages within an allocated region of memory
//
// Arguments:
//
//	address		- Base address of the guard page region
//	length		- Length of the guard page region
//	prot		- Memory protection flags for the region

void Host::GuardMemory(const void* address, size_t length, MemoryProtection prot) const
{
	// Use the common internal version that accepts windows page flags
	return ProtectMemoryInternal(address, length, convert<DWORD>(prot) | PAGE_GUARD);
}
	
//-----------------------------------------------------------------------------
// Host::LockMemory
//
// Attempts to lock a region of data into the process working set, does not throw 
// an exception if it fails
//
// Arguments:
//
//	address		- Base address of the region to be locked
//	length		- Length of the region to be locked

void Host::LockMemory(const void* address, size_t length) const
{
	// Attempt to lock the requested region into the process working set
	LPVOID addr = const_cast<void*>(address);
	NtApi::NtLockVirtualMemory(m_nativeproc->ProcessHandle, &addr, reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);
}

//-----------------------------------------------------------------------------
// Host::ProtectMemory
//
// Assigns memory protection flags for an allocated region of memory
//
// Arguments:
//
//	address		- Base address of the region to be protected
//	length		- Length of the region to be protected
//	prot		- Memory protection flags for the region

void Host::ProtectMemory(const void* address, size_t length, MemoryProtection prot) const
{
	// Use the common internal version that accepts windows page flags
	return ProtectMemoryInternal(address, length, convert<DWORD>(prot));
}

//-----------------------------------------------------------------------------
// Host::ProtectMemoryInternal (private)
//
// Assigns memory protection flags for an allocated region of memory
//
// Arguments:
//
//	address		- Base address of the region to be protected
//	length		- Length of the region to be protected
//	winprot		- Windows memory protection flags for the region

void Host::ProtectMemoryInternal(const void* address, size_t length, DWORD winprot) const
{
	// Determine the starting and ending points for the operation
	uintptr_t begin = uintptr_t(address);
	uintptr_t end = begin + length;

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	while(begin < end) {

		// Locate the section object that matches the current base address
		const auto& found = std::find_if(m_sections.begin(), m_sections.end(), [&](const std::unique_ptr<MemorySection>& section) -> bool {
			return ((begin >= uintptr_t(section->BaseAddress)) && (begin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, throw EINVAL/ERROR_INVALID_ADDRESS
		if(found == m_sections.end()) throw LinuxException{ LINUX_EINVAL, Win32Exception{ ERROR_INVALID_ADDRESS } };

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		const auto& section = *found;

		// Determine the length of the allocation to request from this section and request it
		size_t protectlen = std::min(section->Length - (begin - uintptr_t(section->BaseAddress)), end - begin);
		section->Protect(reinterpret_cast<void*>(begin), protectlen, winprot);

		begin += protectlen;
	}
}

//-----------------------------------------------------------------------------
// Host::ReadMemory
//
// Reads data from the process address space
//
// Arguments:
//
//	address		- Address in the process from which to read
//	buffer		- Output data buffer
//	length		- Size of the output data buffer

size_t Host::ReadMemory(const void* address, void* buffer, size_t length) const
{
	SIZE_T						read;			// Number of bytes read

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Attempt to read the requested data from the native process
	NTSTATUS result = NtApi::NtReadVirtualMemory(m_nativeproc->ProcessHandle, address, buffer, length, &read);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EFAULT, StructuredException(result) };

	return static_cast<size_t>(read);
}

//-----------------------------------------------------------------------------
// Host::ReleaseMemory
//
// Releases memory from the process address space
//
// Arguments:
//
//	address		- Base address of the memory region to be released
//	length		- Number of bytes to be released

void Host::ReleaseMemory(const void* address, size_t length)
{
	uintptr_t begin = uintptr_t(address);			// Start address as uintptr_t
	uintptr_t end = begin + length;					// End address as uintptr_t

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

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
		size_t freelength = std::min(section->Length - (begin - uintptr_t(section->BaseAddress)), end - begin);
		section->Release(reinterpret_cast<void*>(begin), freelength);

		// If the section is empty after the release, remove it from the process
		if(section->Empty) m_sections.erase(found);

		begin += freelength;
	}
}

//-----------------------------------------------------------------------------
// Host::UnlockMemory
//
// Attempts to unlock a region of data from the process working set, does not 
// throw an exception if it fails
//
// Arguments:
//
//	address		- Base address of the region to be unlocked
//	length		- Length of the region to be unlocked

void Host::UnlockMemory(const void* address, size_t length) const
{
	// Attempt to unlock the requested region from the process working set
	LPVOID addr = const_cast<void*>(address);
	NtApi::NtUnlockVirtualMemory(m_nativeproc->ProcessHandle, &addr, reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);
}

//-----------------------------------------------------------------------------
// Host::WriteMemory
//
// Writes data into the process address space
//
// Arguments:
//
//	address		- Address in the client process from which to read
//	buffer		- Output data buffer
//	length		- Size of the output data buffer

size_t Host::WriteMemory(const void* address, const void* buffer, size_t length) const
{
	SIZE_T					written;			// Number of bytes written

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Attempt to write the requested data into the native process
	NTSTATUS result = NtApi::NtWriteVirtualMemory(m_nativeproc->ProcessHandle, address, buffer, length, &written);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EFAULT, StructuredException{ result } };

	return written;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
