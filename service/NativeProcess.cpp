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
#include "NativeProcess.h"

#include <tuple>
#include <vector>
#include "LinuxException.h"
#include "NtApi.h"
#include "StructuredException.h"
#include "SystemInformation.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

// SectionProtection
//
// Alias for ULONG, used with convert<> template
using SectionProtection = ULONG;

// convert<SectionProtection>(ProcessMemory::Protection)
//
// Converts a ProcessMemory::Protection value into Win32 Section protection flags
//
template<> SectionProtection convert<SectionProtection>(ProcessMemory::Protection const& rhs)
{
	using prot = ProcessMemory::Protection;	
	
	prot base(rhs & ~prot::Guard);
	SectionProtection result = PAGE_NOACCESS;

	if(base == prot::Execute)									result = PAGE_EXECUTE;
	else if(base == prot::Read)									result = PAGE_READONLY;
	else if(base == prot::Write)								result = PAGE_READWRITE;
	else if(base == (prot::Execute | prot::Read))				result = PAGE_EXECUTE_READ;
	else if(base == (prot::Execute | prot::Write))				result = PAGE_EXECUTE_READWRITE;
	else if(base == (prot::Read | prot::Write))					result = PAGE_READWRITE;
	else if(base == (prot::Execute | prot::Read | prot::Write))	result = PAGE_EXECUTE_READWRITE;

	return (rhs & prot::Guard) ? result |= PAGE_GUARD : result;
}

//-----------------------------------------------------------------------------
// NativeProcess Constructor
//
// Arguments:
//
//	architecture	- Native process architecture flag
//	procinfo		- Process information structure

NativeProcess::NativeProcess(enum class Architecture architecture, PROCESS_INFORMATION& procinfo) :
	m_architecture(architecture), m_process(procinfo.hProcess), m_processid(procinfo.dwProcessId), m_thread(procinfo.hThread), m_threadid(procinfo.dwThreadId) 
{
}

//-----------------------------------------------------------------------------
// NativeProcess Destructor

NativeProcess::~NativeProcess()
{
	// Release all local mappings and target process section mappings
	for(auto const& iterator : m_localmappings) ReleaseLocalMappings(NtApi::NtCurrentProcess, iterator.second);
	for(auto const& iterator : m_sections) ReleaseSection(m_process, iterator);

	CloseHandle(m_thread);				// Close the main thread handle
	CloseHandle(m_process);				// Close the process handle
}

//-----------------------------------------------------------------------------
// NativeProcess::AllocateMemory
//
// Allocates a region of virtual memory
//
// Arguments:
//
//	length			- Length of the region to allocate
//	protection		- Protection flags to assign to the allocated region

uintptr_t NativeProcess::AllocateMemory(size_t length, ProcessMemory::Protection protection)
{
	ULONG				previous;					// Previous memory protection flags

	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	// Emplace a new section into the section collection, aligning the length up to the allocation granularity
	auto iterator = m_sections.emplace(CreateSection(m_process, uintptr_t(0), align::up(length, SystemInformation::AllocationGranularity)));
	if(!iterator.second) throw LinuxException{ LINUX_ENOMEM };

	// The pages for the section are implicitly committed when mapped, "allocation" merely applies the protection flags
	void* address = reinterpret_cast<void*>(iterator.first->m_baseaddress);
	NTSTATUS result = NtApi::NtProtectVirtualMemory(m_process, reinterpret_cast<void**>(&address), reinterpret_cast<PSIZE_T>(&length), convert<SectionProtection>(protection), &previous);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_ENOMEM, StructuredException{ result } };

	// Track the "allocated" pages in the section's allocation bitmap
	size_t pages = align::up(length, SystemInformation::PageSize) / SystemInformation::PageSize;
	iterator.first->m_allocationmap.Set(0, pages);

	return iterator.first->m_baseaddress;
}

//-----------------------------------------------------------------------------
// NativeProcess::AllocateMemory
//
// Allocates a region of virtual memory
//
// Arguments:
//
//	address			- Base address for the allocation
//	length			- Length of the region to allocate
//	protection		- Protection flags to assign to the allocated region

uintptr_t NativeProcess::AllocateMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection)
{
	// This operation is different when the caller doesn't care what the base address is
	if(address == 0) return AllocateMemory(length, protection);

	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	ReserveRange(writer, address, length);				// Ensure address space is reserved

	// "Allocate" all of the pages in the specified range with the requested protection attributes
	IterateRange(writer, address, length, [=](section_t const& section, uintptr_t address, size_t length) -> void {

		ULONG			previous;						// Previous memory protection flags

		// Section pages are implicitly committed when mapped, just change the protection flags
		NTSTATUS result = NtApi::NtProtectVirtualMemory(m_process, reinterpret_cast<void**>(&address), reinterpret_cast<PSIZE_T>(&length), convert<SectionProtection>(protection), &previous);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };

		// Track the allocated pages in the section's allocation bitmap
		size_t pages = align::up(length, SystemInformation::PageSize) / SystemInformation::PageSize;
		section.m_allocationmap.Set((address - section.m_baseaddress) / SystemInformation::PageSize, pages);
	});

	return address;
}

//-----------------------------------------------------------------------------
// NativeProcess::getArchitecture
//
// Gets the architecture of the native process

enum class Architecture NativeProcess::getArchitecture(void) const
{
	return m_architecture;
}
	
//-----------------------------------------------------------------------------
// NativeProcess::Create
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	path			- Path to the native process executable
//	arguments		- Arguments to pass to the executable

std::unique_ptr<NativeProcess> NativeProcess::Create(const tchar_t* path, const tchar_t* arguments)
{
	return Create(path, arguments, nullptr, 0);
}

//-----------------------------------------------------------------------------
// NativeProcess::Create
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	path			- Path to the native process executable
//	arguments		- Arguments to pass to the executable
//	handles			- Optional array of inheritable handle objects
//	numhandles		- Number of elements in the handles array

std::unique_ptr<NativeProcess> NativeProcess::Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles)
{
	PROCESS_INFORMATION				procinfo;			// Process information

	// If a null argument string was provided, change it to an empty string
	if(arguments == nullptr) arguments = _T("");

	// Generate the command line for the child process, using the specifed path as argument zero
	tchar_t commandline[MAX_PATH];
	_sntprintf_s(commandline, MAX_PATH, MAX_PATH, _T("\"%s\"%s%s"), path, (arguments[0]) ? _T(" ") : _T(""), arguments);

	// Determine the size of the attributes buffer required to hold the inheritable handles property
	SIZE_T required = 0;
	InitializeProcThreadAttributeList(nullptr, 1, 0, &required);
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };

	// Allocate a buffer large enough to hold the attribute data and initialize it
	auto buffer = std::make_unique<uint8_t[]>(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&buffer[0]);
	if(!InitializeProcThreadAttributeList(attributes, 1, 0, &required)) LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };

	try {

		// UpdateProcThreadAttribute will fail if there are no handles in the specified array
		if((handles != nullptr) && (numhandles > 0)) {
			
			// Add the array of handles as inheritable handles for the client process
			if(!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, numhandles * sizeof(HANDLE),
				nullptr, nullptr)) throw LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };
		}

		// Attempt to launch the process using the CREATE_SUSPENDED and EXTENDED_STARTUP_INFO_PRESENT flag
		zero_init<STARTUPINFOEX> startinfo;
		startinfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		startinfo.lpAttributeList = attributes;
		if(!CreateProcess(path, commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr, 
			nullptr, &startinfo.StartupInfo, &procinfo)) throw LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };

		DeleteProcThreadAttributeList(attributes);			// Clean up the PROC_THREAD_ATTRIBUTE_LIST
	}

	catch(...) { DeleteProcThreadAttributeList(attributes); throw; }

	// Process was successfully created and initialized, construct the NativeProcess instance
	try { return std::make_unique<NativeProcess>(GetProcessArchitecture(procinfo.hProcess), procinfo); }

	catch(...) {

		// It's unlikely that the creation of NativeProcess would fail, but if it does clean up
		TerminateProcess(procinfo.hProcess, ERROR_PROCESS_ABORTED);
		CloseHandle(procinfo.hThread);
		CloseHandle(procinfo.hProcess);
		throw;
	}
}

//-----------------------------------------------------------------------------
// NativeProcess::CreateSection (private, static)
//
// Creates a new memory section object and maps it into the process
//
// Arguments:
//
//	process		- Target process handle
//	address		- Base address of the section to be created and mapped
//	length		- Length of the section to be created and mapped

NativeProcess::section_t NativeProcess::CreateSection(HANDLE process, uintptr_t address, size_t length)
{
	HANDLE					section;				// The newly created section handle
	LARGE_INTEGER			sectionlength;			// Section length as a LARGE_INTEGER
	void*					mapping;				// Address of mapped section
	SIZE_T					mappinglength = 0;		// Length of the mapped section view
	ULONG					previous;				// Previously set page protection flags
	NTSTATUS				result;					// Result from function call

	// These values should have been aligned before attempting to create the section object
	_ASSERTE((address % SystemInformation::AllocationGranularity) == 0);
	_ASSERTE((length % SystemInformation::AllocationGranularity) == 0);

	// Create a section of the requested length with an ALL_ACCESS mask and PAGE_EXECUTE_READWRITE protection and commit all pages
	sectionlength.QuadPart = length;
	result = NtApi::NtCreateSection(&section, SECTION_ALL_ACCESS, nullptr, &sectionlength, PAGE_EXECUTE_READWRITE, SEC_COMMIT, nullptr);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_ENOMEM, StructuredException{ result } };

	// Convert the address into a void pointer for NtMapViewOfSection and section_t
	mapping = reinterpret_cast<void*>(address);

	try {

		// Attempt to map the section into the target process' address space with PAGE_EXECUTE_READWRITE as the allowable protection
		result = NtApi::NtMapViewOfSection(section, process, &mapping, 0, 0, nullptr, &mappinglength, NtApi::ViewUnmap, 0, PAGE_EXECUTE_READWRITE);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_ENOMEM, StructuredException{ result } };

		try {

			// The allowable permissions of PAGE_EXECUTE_READWRITE are automatically applied by NtMapViewOfSection to the committed pages,
			// but should be brought back down to PAGE_NOACCESS since no pages in this section are soft-allocated at the time of creation
			result = NtApi::NtProtectVirtualMemory(process, &mapping, reinterpret_cast<PSIZE_T>(&length), PAGE_NOACCESS, &previous);
			if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };

		}

		catch(...) { NtApi::NtUnmapViewOfSection(process, &mapping); throw;  }
	}

	catch(...) { NtApi::NtClose(section); throw; }

	// Return a new section_t structure instance to the caller
	return section_t(section, uintptr_t(mapping), mappinglength);
}

//-----------------------------------------------------------------------------
// NativeProcess::DuplicateHandle (private, static)
//
// Duplicates a Win32 handle with the same attributes and access
//
// Arguments:
//
//	original	- Original Win32 HANDLE to be duplicated

HANDLE NativeProcess::DuplicateHandle(HANDLE original)
{
	HANDLE duplicate = nullptr;

	if(!::DuplicateHandle(GetCurrentProcess(), original, GetCurrentProcess(), &duplicate, 0, FALSE, DUPLICATE_SAME_ACCESS))
		throw LinuxException{ LINUX_EACCES, Win32Exception{ GetLastError() } };

	return duplicate;
}

//-----------------------------------------------------------------------------
// NativeProcess::EnsureSectionAllocation (private, static)
//
// Verifies that the specified address range is soft-allocated within a section
//
// Arguments:
//
//	section		- Section object to check the soft allocations
//	address		- Starting address of the range to check
//	length		- Length of the range to check

inline void NativeProcess::EnsureSectionAllocation(section_t const& section, uintptr_t address, size_t length)
{
	size_t pages = align::up(length, SystemInformation::PageSize) / SystemInformation::PageSize;

	if(!section.m_allocationmap.AreBitsSet((address - section.m_baseaddress) / SystemInformation::PageSize, pages)) 
		throw LinuxException{ LINUX_EACCES, Win32Exception{ ERROR_INVALID_ADDRESS } };
}

//-----------------------------------------------------------------------------
// NativeProcess::getExitCode
//
// Gets the exit code of the process, or STILL_ACTIVE if it's still running

DWORD NativeProcess::getExitCode(void) const
{
	DWORD				result;				// Result from GetExitCodeProcess

	if(!GetExitCodeProcess(m_process, &result)) throw Win32Exception{};

	return result;
}

//-----------------------------------------------------------------------------
// NativeProcess::GetProcessArchitecture (private, static)
//
// Determines the Architecture of a native process
//
// Arguments:
//
//	process		- Native process handle

enum class Architecture NativeProcess::GetProcessArchitecture(HANDLE process)
{
	BOOL				result;				// Result from IsWow64Process

	// If the operating system is 32-bit, the architecture must be x86
	if(SystemInformation::ProcessorArchitecture == SystemInformation::Architecture::Intel) return Architecture::x86;

	// 64-bit operating system, check the WOW64 status of the process to determine architecture
	if(!IsWow64Process(process, &result)) throw Win32Exception{};

	return (result) ? Architecture::x86 : Architecture::x86_64;
}

//-----------------------------------------------------------------------------
// NativeProcess::IterateRange (private)
//
// Iterates across an address range and invokes the specified operation for each section, this
// ensures that the range is managed by this implementation and allows for operations that do 
// not operate across sections (allocation, release, protection, etc)
//
// Arguments:
//
//	lock		- Reference to a scoped_lock (unused, ensures caller has locked m_sections)
//	start		- Starting address of the range to iterate over
//	length		- Length of the range to iterate over
//	operation	- Operation to execute against each section in the range individually

void NativeProcess::IterateRange(sync::reader_writer_lock::scoped_lock& lock, uintptr_t start, size_t length, sectioniterator_t operation) const
{
	UNREFERENCED_PARAMETER(lock);				// This is just to ensure the caller has locked m_sections

	uintptr_t end = start + length;				// Determine the range ending address
	auto iterator = m_sections.begin();			// Start at the beginning of the reserved sections

	while((start < end) && (iterator != m_sections.end())) {

		// If the starting address is lower than the current section, it has not been reserved
		if(start < iterator->m_baseaddress) throw LinuxException{ LINUX_EACCES, Win32Exception{ ERROR_INVALID_ADDRESS } };

		// If the starting address is beyond the end of the current section, move to the next section
		else if(start >= (iterator->m_baseaddress + iterator->m_length)) ++iterator;

		// Starting address is within the current section, process up to the end of the section
		// or the specified address range end, whichever is the lower address, and advance start
		else {

			operation(*iterator, start, std::min(iterator->m_baseaddress + iterator->m_length, end) - start);
			start = iterator->m_baseaddress + iterator->m_length;
		}
	}

	// If any address space was left unprocessed, it has not been reserved
	if(start < end) throw LinuxException{ LINUX_EACCES, Win32Exception{ ERROR_INVALID_ADDRESS } };
}

//-----------------------------------------------------------------------------
// NativeProcess::LockMemory
//
// Attempts to lock a region into physical memory
//
// Arguments:
//
//	address		- Starting address of the region to lock
//	length		- Length of the region to lock

void NativeProcess::LockMemory(uintptr_t address, size_t length) const
{
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Attempt to unlock all pages within the specified address range
	IterateRange(reader, address, length, [=](section_t const& section, uintptr_t address, size_t length) -> void {

		EnsureSectionAllocation(section, address, length);		// All pages must be marked as allocated

		// Attempt to lock the specified pages into physical memory
		NTSTATUS result = NtApi::NtLockVirtualMemory(m_process, reinterpret_cast<void**>(&address), reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };
	});
}

//-----------------------------------------------------------------------------
// NativeProcess::MapMemory
//
// Maps a virtual memory region into the calling process.  Note that if the operation spans
// multiple sections the operation may fail fairly easily since each section will be mapped
// contiguously, after the operating system chooses the address for the first one.  There is
// no guarantee that the subsequent address space will be available.  For this reason it is
// recommended to only map known single section ranges and use Read/Write other times
//
// Arguments:
//
//	address		- Starting address of the region to map (relative to target process)
//	length		- Length of the region to map
//	protection	- Protection flags to assign to the region after it's been mapped

void* NativeProcess::MapMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection)
{
	std::vector<uintptr_t>		mappings;					// Created section mappings
	void*						nextmapping = nullptr;		// Address of the next mapping
	void*						returnptr = nullptr;		// Pointer to return to the caller

	// Guard pages cannot be specified as the protection for this function
	if(protection & ProcessMemory::Protection::Guard) throw LinuxException{ LINUX_EINVAL };

	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	//
	// TODO: This doesn't necessarily need to map entire section(s) into the local process,
	// the offsets and lengths are known.  The complication comes in when multiple sections
	// need to be mapped -- the boundary between them must fall on a 64K alignment boundary
	//

	try {

		IterateRange(writer, address, length, [&](section_t const& section, uintptr_t address, size_t length) -> void {

			SIZE_T mappedlength = 0;								// Length of section mapped by NtMapViewOfSection
			EnsureSectionAllocation(section, address, length);		// All pages must be marked as allocated

			// Attempt to map the entire section into the current process' address space.  The first iteration will allow the operating
			// system to select the destination address, subsequent operations are mapped contiguously with the previous one
			NTSTATUS result = NtApi::NtMapViewOfSection(section.m_section, NtApi::NtCurrentProcess, &nextmapping, 0, 0, nullptr, &mappedlength, NtApi::ViewUnmap, 0, convert<SectionProtection>(protection));
			if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };

			// The first time through calculate the pointer to return to the caller, which is an offset into this first mapping
			if(returnptr == nullptr) returnptr = reinterpret_cast<void*>(uintptr_t(nextmapping) + (address - section.m_baseaddress));

			// Track the base address of the new mapping and determine the address where the next one needs to be placed
			mappings.emplace_back(uintptr_t(nextmapping));
			nextmapping = reinterpret_cast<void*>(uintptr_t(nextmapping) + mappedlength);
		});

		// Track the local mapping via the pointer that the caller will receive and return that pointer
		m_localmappings.emplace(returnptr, std::move(mappings));
		return returnptr;
	}

	catch(...) { ReleaseLocalMappings(NtApi::NtCurrentProcess, mappings); throw; }
}

//-----------------------------------------------------------------------------
// NativeProcess::getProcessHandle
//
// Gets the host process handle

HANDLE NativeProcess::getProcessHandle(void) const
{
	return m_process;
}

//-----------------------------------------------------------------------------
// NativeProcess::getProcessId
//
// Gets the host process identifier

DWORD NativeProcess::getProcessId(void) const
{
	return m_processid;
}

//-----------------------------------------------------------------------------
// NativeProcess::ProtectMemory
//
// Sets the memory protection flags for a virtual memory region
//
// Arguments:
//
//	address		- Starting address of the region to protect
//	length		- Length of the region to protect
//	protection	- Protection flags to assign to the region

void NativeProcess::ProtectMemory(uintptr_t address, size_t length, ProcessMemory::Protection protection) const
{
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Set the protection for all of the pages in the specified range
	IterateRange(reader, address, length, [=](section_t const& section, uintptr_t address, size_t length) -> void {

		ULONG previous = 0;										// Previously set protection flags
		EnsureSectionAllocation(section, address, length);		// All pages must be marked as allocated

		// Apply the specified protection flags to the region
		NTSTATUS result = NtApi::NtProtectVirtualMemory(m_process, reinterpret_cast<void**>(&address), reinterpret_cast<PSIZE_T>(&length), convert<SectionProtection>(protection), &previous);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };
	});
}

//-----------------------------------------------------------------------------
// NativeProcess::ReadMemory
//
// Reads data from a virtual memory region into the calling process
//
// Arguments:
//
//	address		- Starting address from which to read
//	buffer		- Destination buffer
//	length		- Number of bytes to read from the process buffer

size_t NativeProcess::ReadMemory(uintptr_t address, void* buffer, size_t length) const
{
	size_t					total = 0;				// Number of bytes read from the process

	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Execute the read operation in multiple steps as necessary to ensure all addresses are "allocated"
	IterateRange(reader, address, length, [&](section_t const& section, uintptr_t address, size_t length) -> void {

		SIZE_T read = 0;										// Number of bytes read from the process
		EnsureSectionAllocation(section, address, length);		// All pages must be marked as allocated

		// Attempt to read the next chunk of virtual memory from the target process' address space
		NTSTATUS result = NtApi::NtReadVirtualMemory(m_process, reinterpret_cast<void*>(address), buffer, length, &read);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };

		// Increment the total number of bytes read as well as the buffer pointer
		total += read;
		buffer = reinterpret_cast<void*>(uintptr_t(buffer) + read);
	});

	return total;
}

//-----------------------------------------------------------------------------
// NativeProcess::ReleaseMemory
//
// Releases a virtual memory region
//
// Arguments:
//
//	address		- Base address of the region to be released
//	length		- Length of the region to be released

void NativeProcess::ReleaseMemory(uintptr_t address, size_t length)
{
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	// Release all of the pages in the specified range
	IterateRange(writer, address, length, [=](section_t const& section, uintptr_t address, size_t length) -> void {

		ULONG previous;				// Previously set protection flags for this range of pages

		// Attempt to change the protection of the pages involved to PAGE_NOACCESS since they can't be decommitted
		NTSTATUS result = NtApi::NtProtectVirtualMemory(m_process, reinterpret_cast<void**>(&address), reinterpret_cast<PSIZE_T>(&length), PAGE_NOACCESS, &previous);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };

		// Unlock the pages from physical memory (this operation will typically fail, don't bother checking result)
		NtApi::NtUnlockVirtualMemory(m_process, reinterpret_cast<void**>(&address), reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);

		// Clear the corresponding pages from the section allocation bitmap to indicate they are "released"
		size_t pages = align::up(length, SystemInformation::PageSize) / SystemInformation::PageSize;
		section.m_allocationmap.Clear((address - section.m_baseaddress) / SystemInformation::PageSize, pages);
	});

	// Remove any sections that are now completely empty to actually release and unmap that memory
	auto iterator = m_sections.begin();
	while(iterator != m_sections.end()) {

		if(iterator->m_allocationmap.Empty) {

			ReleaseSection(m_process, *iterator);
			iterator = m_sections.erase(iterator);
		}

		else ++iterator;
	}
}

//-----------------------------------------------------------------------------
// NativeProcess::ReleaseLocalMappings (private, static)
//
// Releases mappings contained in a vector of base addresses
//
// Arguments:
//
//	process		- Target process handle
//	mappings	- Vector of mapping base addresses to release

inline void NativeProcess::ReleaseLocalMappings(HANDLE process, std::vector<uintptr_t> const& mappings)
{
	_ASSERTE(process == NtApi::NtCurrentProcess);
	for(auto const& iterator : mappings) NtApi::NtUnmapViewOfSection(process, reinterpret_cast<void*>(iterator));
}

//-----------------------------------------------------------------------------
// NativeProcess::ReleaseSection (private, static)
//
// Releases a section represented by a section_t instance
//
// Arguments:
//
//	process		- Target process handle
//	section		- Reference to the section to be released

inline void NativeProcess::ReleaseSection(HANDLE process, section_t const& section)
{
	NtApi::NtUnmapViewOfSection(process, reinterpret_cast<void*>(section.m_baseaddress));
	NtApi::NtClose(section.m_section);
}

//-----------------------------------------------------------------------------
// NativeProcess::ReserveMemory
//
// Reserves a virtual memory region for later allocation
//
// Arguments:
//
//	length		- Length of the memory region to reserve

uintptr_t NativeProcess::ReserveMemory(size_t length)
{
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	// Emplace a new section into the section collection, aligning the length up to the allocation granularity
	auto iterator = m_sections.emplace(CreateSection(m_process, uintptr_t(0), align::up(length, SystemInformation::AllocationGranularity)));

	if(!iterator.second) throw LinuxException{ LINUX_ENOMEM };
	return iterator.first->m_baseaddress;
}

//-----------------------------------------------------------------------------
// NativeProcess::ReserveMemory
//
// Reserves a virtual memory region for later allocation
//
// Arguments:
//
//	address		- Base address of the region to be reserved
//	length		- Length of the region to be reserved

uintptr_t NativeProcess::ReserveMemory(uintptr_t address, size_t length)
{
	// This operation is different when the caller doesn't care what the base address is
	if(address == 0) return ReserveMemory(length);

	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	ReserveRange(writer, address, length);			// Ensure range is reserved
	return address;									// Return original address
}

//-----------------------------------------------------------------------------
// NativeProcess::ReserveRange (private)
//
// Ensures that a range of address space is reserved
//
//	writer		- Reference to a scoped_lock_write (unused, ensures caller has locked m_sections)
//	address		- Starting address of the range to be reserved
//	length		- Length of the range to be reserved

void NativeProcess::ReserveRange(sync::reader_writer_lock::scoped_lock_write& writer, uintptr_t address, size_t length)
{
	UNREFERENCED_PARAMETER(writer);				// This is just to ensure the caller has locked m_sections

	// Align the address and length system allocation granularity boundaries
	uintptr_t start = align::down(address, SystemInformation::AllocationGranularity);
	length = align::up(address + length, SystemInformation::AllocationGranularity) - start;

	uintptr_t end = start + length;				// Determine the range ending address (aligned)
	auto iterator = m_sections.cbegin();		// Start at the beginning of the reserved sections

	// Iterate over the existing sections to look for gaps that need to be filled in with reservations
	while((iterator != m_sections.end()) && (start < end)) {

		// If the start address is lower than the current section, fill the region with a new reservation
		if(start < iterator->m_baseaddress) {

			m_sections.emplace(CreateSection(m_process, start, std::min(end, iterator->m_baseaddress) - start));
			start = (iterator->m_baseaddress + iterator->m_length);
		}

		// If the start address falls within this section, move to the end of this reservation
		else if(start < (iterator->m_baseaddress + iterator->m_length)) start = (iterator->m_baseaddress + iterator->m_length);

		++iterator;								// Move to the next section
	}

	// After all the sections have been examined, create a final section if necessary
	if(start < end) m_sections.emplace(CreateSection(m_process, start, end - start));
}

//-----------------------------------------------------------------------------
// NativeProcess::UnlockMemory
//
// Attempts to unlock a region from physical memory
//
// Arguments:
//
//	address		- Starting address of the region to unlock
//	length		- Length of the region to unlock

void NativeProcess::UnlockMemory(uintptr_t address, size_t length) const
{
	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Attempt to unlock all pages within the specified address range
	IterateRange(reader, address, length, [=](section_t const& section, uintptr_t address, size_t length) -> void {

		EnsureSectionAllocation(section, address, length);		// All pages must be marked as allocated

		// Attempt to unlock the specified pages from physical memory
		NTSTATUS result = NtApi::NtUnlockVirtualMemory(m_process, reinterpret_cast<void**>(&address), reinterpret_cast<PSIZE_T>(&length), NtApi::MAP_PROCESS);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };
	});
}

//-----------------------------------------------------------------------------
// NativeProcess::UnmapMemory
//
// Unmaps a previously mapped memory region from the calling process
//
// Arguments:
//
//	mapping		- Address returned from a successful call to Map

void NativeProcess::UnmapMemory(void const* mapping)
{
	sync::reader_writer_lock::scoped_lock_write writer(m_sectionslock);

	// Locate the mapping address in the local mappings collection
	auto item = m_localmappings.find(mapping);
	if(item == m_localmappings.end()) throw LinuxException{ LINUX_EACCES, Win32Exception{ ERROR_INVALID_ADDRESS } };

	// Iterate over all of the base mapping addresses and release them
	ReleaseLocalMappings(NtApi::NtCurrentProcess, item->second);

	// Remove the local mapping from the collection
	m_localmappings.erase(item);
}

//-----------------------------------------------------------------------------
// NativeProcess::Terminate
//
// Terminates the native process
//
// Arguments:
//
//	exitcode		- Exit code for the process

void NativeProcess::Terminate(uint16_t exitcode) const
{
	Terminate(exitcode, true);		// Wait for the process to terminate
}

//-----------------------------------------------------------------------------
// NativeProcess::Terminate
//
// Terminates the native process
//
// Arguments:
//
//	exitcode		- Exit code for the process
//	wait			- Flag to wait for the process to exit

void NativeProcess::Terminate(uint16_t exitcode, bool wait) const
{
	TerminateProcess(m_process, static_cast<UINT>(exitcode));
	if(wait) WaitForSingleObject(m_process, INFINITE);
}

//-----------------------------------------------------------------------------
// NativeProcess::getThreadHandle
//
// Gets the host main thread handle

HANDLE NativeProcess::getThreadHandle(void) const
{
	return m_thread;
}

//-----------------------------------------------------------------------------
// NativeProcess::getThreadId
//
// Gets the host main thread identifier

DWORD NativeProcess::getThreadId(void) const
{
	return m_threadid;
}
	
//-----------------------------------------------------------------------------
// NativeProcess::WriteMemory
//
// Writes data into a virtual memory region from the calling process
//
// Arguments:
//
//	address		- Starting address from which to write
//	buffer		- Source buffer
//	length		- Number of bytes to write into the process

size_t NativeProcess::WriteMemory(uintptr_t address, void const* buffer, size_t length) const
{
	size_t					total = 0;				// Number of bytes read from the process

	sync::reader_writer_lock::scoped_lock_read reader(m_sectionslock);

	// Execute the write operation in multiple steps as necessary to ensure all addresses are "allocated"
	IterateRange(reader, address, length, [&](section_t const& section, uintptr_t address, size_t length) -> void {

		SIZE_T written = 0;										// Number of bytes written to the process
		EnsureSectionAllocation(section, address, length);		// All pages must be marked as allocated

		// Attempt to write the next chunk of data into the target process' virtual address space
		NTSTATUS result = NtApi::NtWriteVirtualMemory(m_process, reinterpret_cast<void const*>(address), buffer, length, &written);
		if(result != NtApi::STATUS_SUCCESS) throw LinuxException{ LINUX_EACCES, StructuredException{ result } };

		// Increment the total number of bytes written as well as the buffer pointer
		total += written;
		buffer = reinterpret_cast<void const*>(uintptr_t(buffer) + written);
	});

	return total;
}

//
// __PROCESSMEMORY::SECTION_T IMPLEMENTATION
//

//-----------------------------------------------------------------------------
// NativeProcess::section_t Constructor
//
// Arguments:
//
//	section			- Section object handle
//	baseaddress		- Mapping base address
//	length			- Section/mapping length

NativeProcess::section_t::section_t(HANDLE section, uintptr_t baseaddress, size_t length) : m_section(section), m_baseaddress(baseaddress), 
	m_length(length), m_allocationmap(length / SystemInformation::PageSize)
{
}

//-----------------------------------------------------------------------------
// NativeProcess::section_t::operator <

bool NativeProcess::section_t::operator <(section_t const& rhs) const
{
	return m_baseaddress < rhs.m_baseaddress;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
