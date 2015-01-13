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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Host Destructor

Host::~Host()
{
	CloseHandle(m_procinfo.hThread);
	CloseHandle(m_procinfo.hProcess);
}

//------------------------------------------------------------------------------
// Host::AllocateMemory
//
// Allocates memory in the native operating system process
//
// Arguments:
//
//	address			- Optional base address or nullptr if it doesn't matter
//	length			- Required allocation length
//	protection		- Native memory protection flags for the new region

const void* Host::AllocateMemory(const void* address, size_t length, DWORD protection)
{
	MEMORY_BASIC_INFORMATION					meminfo;		// Virtual memory information

	// Allocations cannot be zero-length
	if(length == 0) throw Win32Exception(ERROR_INVALID_PARAMETER);

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock writer(m_sectionlock);

	// No specific address was requested, let the operating system decide where it should go
	if(address == nullptr) {

		std::unique_ptr<MemorySection> section = MemorySection::Create(m_procinfo.hProcess, align::up(length, SystemInformation::AllocationGranularity));
		address = section->Allocate(section->BaseAddress, length, protection);
		m_sections.push_back(std::move(section));
		return address;
	}

	// A specific address was requested, first scan over the virtual address space and fill in any holes
	// with new meory sections to ensure a contiguous region
	uintptr_t fillbegin = align::down(uintptr_t(address), SystemInformation::AllocationGranularity);
	uintptr_t fillend = align::up((uintptr_t(address) + length), SystemInformation::AllocationGranularity);

	while(fillbegin < fillend) {

		// Query the information about the virtual memory beginning at the current address
		if(!VirtualQueryEx(m_procinfo.hProcess, reinterpret_cast<void*>(fillbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) throw Win32Exception();

		// If the region is free (MEM_FREE), create a new memory section in the free space
		if(meminfo.State == MEM_FREE) {

			size_t filllength = min(meminfo.RegionSize, align::up(fillend - fillbegin, SystemInformation::AllocationGranularity));
			m_sections.emplace_back(MemorySection::Create(m_procinfo.hProcess, meminfo.BaseAddress, filllength));
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
		section->Allocate(reinterpret_cast<void*>(allocbegin), alloclen, protection);

		allocbegin += alloclen;
	}

	return address;					// Return the originally requested address
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
	section_lock_t::scoped_lock writer(m_sectionlock);

	// Clearing the vector<> will release all of the section instances
	m_sections.clear();
}

//-----------------------------------------------------------------------------
// Host::CloneMemory
//
// Clones the virtual memory from an existing Host instance
//
// Arguments:
//
//	existing		- Reference to an existing host instance

void Host::CloneMemory(const std::unique_ptr<Host>& existing)
{
	// Prevent changes to the existing process memory layout
	section_lock_t::scoped_lock_read reader(existing->m_sectionlock);

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock writer(m_sectionlock);

	// Iterate over the existing sections and clone them into this native process
	for(auto iterator = existing->m_sections.begin(); iterator != existing->m_sections.end(); iterator++) {

		// Clone the existing memory section with copy-on-write access to it
		m_sections.push_back(MemorySection::FromSection(*iterator, m_procinfo.hProcess, MemorySection::Mode::CopyOnWrite));

		// TODO: DO I WANT TO DO THIS HERE OR HAVE A SEPARATE FUNCTION CALL
		// doing it here is the safest way, but may not be flexible enough in the long run
		(*iterator)->ChangeMode(MemorySection::Mode::CopyOnWrite);
	}
}

//-----------------------------------------------------------------------------
// Host::Create (static)
//
// Creates a new native operating system process instance
//
// Arguments:
//
//	path			- Path to the host binary
//	arguments		- Arguments to pass to the host binary
//	handles			- Optional array of inheritable handle objects
//	numhandles		- Number of elements in the handles array

std::unique_ptr<Host> Host::Create(const tchar_t* path, const tchar_t* arguments, HANDLE handles[], size_t numhandles)
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
	if(GetLastError() != ERROR_INSUFFICIENT_BUFFER) throw Win32Exception();

	// Allocate a buffer large enough to hold the attribute data and initialize it
	HeapBuffer<uint8_t> buffer(required);
	PPROC_THREAD_ATTRIBUTE_LIST attributes = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(&buffer);
	if(!InitializeProcThreadAttributeList(attributes, 1, 0, &required)) throw Win32Exception();

	try {

		// UpdateProcThreadAttribute will fail if there are no handles in the specified array
		if((handles != nullptr) && (numhandles > 0)) {
			
			// Add the array of handles as inheritable handles for the client process
			if(!UpdateProcThreadAttribute(attributes, 0, PROC_THREAD_ATTRIBUTE_HANDLE_LIST, handles, numhandles * sizeof(HANDLE), 
				nullptr, nullptr)) throw Win32Exception();
		}

		// Attempt to launch the process using the CREATE_SUSPENDED and EXTENDED_STARTUP_INFO_PRESENT flag
		zero_init<STARTUPINFOEX> startinfo;
		startinfo.StartupInfo.cb = sizeof(STARTUPINFOEX);
		startinfo.lpAttributeList = attributes;
		if(!CreateProcess(path, commandline, nullptr, nullptr, TRUE, CREATE_SUSPENDED | EXTENDED_STARTUPINFO_PRESENT, nullptr, 
			nullptr, &startinfo.StartupInfo, &procinfo)) throw Win32Exception();

		DeleteProcThreadAttributeList(attributes);			// Clean up the PROC_THREAD_ATTRIBUTE_LIST
	}

	catch(...) { DeleteProcThreadAttributeList(attributes); throw; }

	// Process was successfully created and initialized, pass it off to a Host instance
	return std::make_unique<Host>(procinfo);
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
//	prot		- New native memory protection flags for the region

void Host::ProtectMemory(const void* address, size_t length, DWORD protection)
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
		section->Protect(reinterpret_cast<void*>(begin), protectlen, protection);

		begin += protectlen;
	}
}

//-----------------------------------------------------------------------------
// Host::ReadMemory
//
// Reads from the native operating system process virtual memory
//
// Arguments:
//
//	address		- Address in the client process from which to read
//	buffer		- Local output buffer
//	length		- Size of the local output buffer, maximum bytes to read

size_t Host::ReadMemory(const void* address, void* buffer, size_t length)
{
	SIZE_T						read;			// Number of bytes read

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock_read reader(m_sectionlock);

	// Attempt to read the requested data from the native process
	NTSTATUS result = NtApi::NtReadVirtualMemory(m_procinfo.hProcess, address, buffer, length, &read);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	return static_cast<size_t>(read);
}

//-----------------------------------------------------------------------------
// Host::ReleaseMemory
//
// Releases memory from the native process address space
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
// Host::Resume
//
// Resumes the native operating system process
//
// Arguments:
//
//	NONE

void Host::Resume(void)
{
	NTSTATUS result = NtApi::NtResumeProcess(m_procinfo.hProcess);
	if(result != 0) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// Host::Suspend
//
// Suspends the native operating system process
//
// Arguments:
//
//	NONE

void Host::Suspend(void)
{
	NTSTATUS result = NtApi::NtSuspendProcess(m_procinfo.hProcess);
	if(result != 0) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// Host::Terminate
//
// Terminates the native operating system process
//
// Arguments:
//
//	exitcode	- Exit code to use for process termination

void Host::Terminate(HRESULT exitcode)
{
	if(!TerminateProcess(m_procinfo.hProcess, static_cast<UINT>(exitcode))) throw Win32Exception();
}

//-----------------------------------------------------------------------------
// Host::WriteMemory
//
// Writes into the native operating system process virtual memory
//
// Arguments:
//
//	address		- Address in the client process from which to read
//	buffer		- Local output buffer
//	length		- Size of the local output buffer, maximum bytes to read

size_t Host::WriteMemory(const void* address, const void* buffer, size_t length)
{
	SIZE_T					written;			// Number of bytes written

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Prevent changes to the process memory layout while this is operating
	section_lock_t::scoped_lock_read reader(m_sectionlock);

	// Attempt to write the requested data into the native process
	NTSTATUS result = NtApi::NtWriteVirtualMemory(m_procinfo.hProcess, address, buffer, length, &written);
	if(result != NtApi::STATUS_SUCCESS) throw StructuredException(result);

	return written;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
