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
Host::MemoryProtection const Host::MemoryProtection::Atomic{ LINUX_PROT_SEM };

// Host::MemoryProtection::Execute (static)
//
Host::MemoryProtection const Host::MemoryProtection::Execute{ LINUX_PROT_EXEC };

// Host::MemoryProtection::None (static)
//
Host::MemoryProtection const Host::MemoryProtection::None{ LINUX_PROT_NONE };

// Host::MemoryProtection::Read (static)
//
Host::MemoryProtection const Host::MemoryProtection::Read{ LINUX_PROT_READ };

// Host::MemoryProtection::Write (static)
//
Host::MemoryProtection const Host::MemoryProtection::Write{ LINUX_PROT_WRITE };

//-----------------------------------------------------------------------------
// Conversions
//-----------------------------------------------------------------------------

// Win32MemoryProtection
//
// Typedef to clarify conversions from linux to windows protection flags
using Win32MemoryProtection = DWORD;

// Host::MemoryProtection --> Win32MemoryProtection
//
// Converts MemoryProtection bitmask into the closest equivalent Win32 API bitmask
template<> Win32MemoryProtection convert<Win32MemoryProtection>(Host::MemoryProtection const& rhs)
{
	using prot = Host::MemoryProtection;

	prot flags{ rhs & (prot::Execute | prot::Write | prot::Read) };

	if(flags == prot::Execute)										return PAGE_EXECUTE;
	else if(flags == prot::Read)									return PAGE_READONLY;
	else if(flags == prot::Write)									return PAGE_READWRITE;
	else if(flags == (prot::Execute | prot::Read))					return PAGE_EXECUTE_READ;
	else if(flags == (prot::Execute | prot::Write))					return PAGE_EXECUTE_READWRITE;
	else if(flags == (prot::Read | prot::Write))					return PAGE_READWRITE;
	else if(flags == (prot::Execute | prot::Read | prot::Write))	return PAGE_EXECUTE_READWRITE;

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

void const* Host::AllocateMemory(size_t length, MemoryProtection prot)
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

void const* Host::AllocateMemory(void const* address, size_t length, MemoryProtection prot)
{
	MEMORY_BASIC_INFORMATION					meminfo;		// Virtual memory information

	// Allocations cannot be zero-length
	if(length == 0) throw LinuxException{ LINUX_EINVAL };

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	// No specific address was requested, let the operating system decide where it should go
	if(address == nullptr) {

		auto section = MemorySection::Create(m_nativeproc->ProcessHandle, align::up(length, SystemInformation::AllocationGranularity));
		address = section->Allocate(section->BaseAddress, length, convert<Win32MemoryProtection>(prot));
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
		auto const& found = std::find_if(m_sections.begin(), m_sections.end(), [&](std::unique_ptr<MemorySection> const& section) -> bool {
			return ((allocbegin >= uintptr_t(section->BaseAddress)) && (allocbegin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, throw EINVAL/ERROR_INVALID_ADDRESS
		if(found == m_sections.end()) throw LinuxException{ LINUX_EINVAL, Win32Exception{ ERROR_INVALID_ADDRESS } };

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		auto const& section = *found;

		// Determine the length of the allocation to request from this section and request it
		size_t alloclen = std::min(section->Length - (allocbegin - uintptr_t(section->BaseAddress)), allocend - allocbegin);
		section->Allocate(reinterpret_cast<void*>(allocbegin), alloclen, convert<Win32MemoryProtection>(prot));

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
// Host::Clone
//
// Clones this host instance into another NativeProcess instance
//
// Arguments:
//
//	nativeproc		- NativeProcess instance to take ownership of

std::unique_ptr<Host> Host::Clone(std::unique_ptr<NativeProcess> nativeproc)
{
	section_vector_t		sections;			// Cloned memory sections

	// I'm not really sure how this is going to work across architectures, disallow it
	_ASSERTE(nativeproc->Architecture == m_nativeproc->Architecture);
	if(nativeproc->Architecture != m_nativeproc->Architecture) throw LinuxException{ LINUX_ENOEXEC };

	// Prevent changes to the existing process memory layout
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Iterate over the existing memory sections and clone them into the target process
	for(auto const& iterator : m_sections) sections.emplace_back(iterator->Clone(nativeproc->ProcessHandle));

	// Create the new Host instance with the cloned memory sections
	return std::make_unique<Host>(std::move(nativeproc), std::move(sections));
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

void Host::GuardMemory(void const* address, size_t length, MemoryProtection prot) const
{
	// Use the common internal version that accepts windows page flags
	return ProtectMemoryInternal(address, length, convert<Win32MemoryProtection>(prot) | PAGE_GUARD);
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

void Host::LockMemory(void const* address, size_t length) const
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

void Host::ProtectMemory(void const* address, size_t length, MemoryProtection prot) const
{
	// Use the common internal version that accepts windows page flags
	return ProtectMemoryInternal(address, length, convert<Win32MemoryProtection>(prot));
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

void Host::ProtectMemoryInternal(void const* address, size_t length, DWORD winprot) const
{
	// Determine the starting and ending points for the operation
	uintptr_t begin = uintptr_t(address);
	uintptr_t end = begin + length;

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	while(begin < end) {

		// Locate the section object that matches the current base address
		auto const& found = std::find_if(m_sections.begin(), m_sections.end(), [&](std::unique_ptr<MemorySection> const& section) -> bool {
			return ((begin >= uintptr_t(section->BaseAddress)) && (begin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, throw EINVAL/ERROR_INVALID_ADDRESS
		if(found == m_sections.end()) throw LinuxException{ LINUX_EINVAL, Win32Exception{ ERROR_INVALID_ADDRESS } };

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		auto const& section = *found;

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

size_t Host::ReadMemory(void const* address, void* buffer, size_t length) const
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
// Host::ReadMemoryInto
//
// Reads data from the process address space into a file system handle
//
// Arguments:
//
//	handle		- File system object handle
//	offset		- Offset within the file to write the data
//	address		- Adress in the client process to read from
//	length		- Number of bytes to from the client process

size_t Host::ReadMemoryInto(std::shared_ptr<FileSystem::Handle> handle, size_t offset, void const* address, size_t length) const
{
	// todo
	_ASSERTE(false);

	// See comments in WriteMemoryFrom() as well

	(handle);
	(offset);
	(address);
	(length);
	
	return 0;
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

void Host::ReleaseMemory(void const* address, size_t length)
{
	uintptr_t begin = uintptr_t(address);			// Start address as uintptr_t
	uintptr_t end = begin + length;					// End address as uintptr_t

	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	while(begin < end) {

		// Locate the section object that matches the specified base address
		auto const& found = std::find_if(m_sections.begin(), m_sections.end(), [&](std::unique_ptr<MemorySection> const& section) -> bool {
			return ((begin >= uintptr_t(section->BaseAddress)) && (begin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, treat this as a no-op
		if(found == m_sections.end()) return;

		// Cast out the std::unique_ptr<MemorySection>& for clarity below
		auto const& section = *found;

		// Determine how much to release from this section and release it
		size_t freelength = std::min(section->Length - (begin - uintptr_t(section->BaseAddress)), end - begin);
		section->Release(reinterpret_cast<void*>(begin), freelength);

		// If the section is empty after the release, remove it from the process
		if(section->Empty) m_sections.erase(found);

		begin += freelength;
	}
}

//-----------------------------------------------------------------------------
// Host::Terminate
//
// Terminates the host process
//
// Arguments:
//
//	exitcode		- Exit code for the process

void Host::Terminate(uint16_t exitcode) const
{
	m_nativeproc->Terminate(exitcode);
}

//-----------------------------------------------------------------------------
// Host::Terminate
//
// Terminates the host process
//
// Arguments:
//
//	exitcode		- Exit code for the process
//	wait			- Flag to wait for the process to exit

void Host::Terminate(uint16_t exitcode, bool wait) const
{
	m_nativeproc->Terminate(exitcode, wait);
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

void Host::UnlockMemory(void const* address, size_t length) const
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
//	address		- Address in the client process to begin writing data
//	buffer		- Output data buffer
//	length		- Size of the output data buffer

size_t Host::WriteMemory(void const* address, void const* buffer, size_t length) const
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
// Host::WriteMemoryFrom
//
// Writes data into the process from a file system object handle
//
// Arguments:
//
//	handle		- File system object handle
//	offset		- Offset within the file to read the data
//	address		- Adress in the client process to begin writing data
//	length		- Number of bytes to write into the client process

size_t Host::WriteMemoryFrom(std::shared_ptr<FileSystem::Handle> handle, size_t offset, void const* address, size_t length) const
{
	uintptr_t			dest = uintptr_t(address);			// Easier pointer math as uintptr_t
	SIZE_T				written;							// Number of bytes written into process
	size_t				total = 0;							// Total bytes written
	
	// Prevent changes to the process memory layout while this is operating
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	//
	// TODO: This would perform better if the memory was mapped directly, which is something
	// that certain file systems (tmpfs, hostfs) would be able to do at some point.  Perhaps
	// a flag on Handle, like if(handle->CanAddress) pointer = handle->GetAddress(offset) or
	// something like [void* Handle::MapRegion(offset, length)]?  Would need to be performance
	// tested to see if it's really any better than just reading into an intermediate buffer
	//

	// This function seems to perform the best with allocation granularity chunks of data (64KiB)
	auto buffer = std::make_unique <uint8_t[]>(SystemInformation::AllocationGranularity);

	while(length) {

		// Read the next chunk of memory into the heap buffer, break early if there is no more
		size_t read = handle->ReadAt(offset + total, &buffer[0], std::min(length, SystemInformation::AllocationGranularity));
		if(read == 0) break;

		// Attempt to write the requested data into the native process
		NTSTATUS result = NtApi::NtWriteVirtualMemory(m_nativeproc->ProcessHandle, reinterpret_cast<void*>(dest + total), &buffer[0], read, &written);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EFAULT, StructuredException{ result } };
		
		length -= written;		// Subtract number of bytes written from remaining
		total += written;		// Add to the total number of bytes written
	};

	return total;				// Return total bytes written into the process
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
