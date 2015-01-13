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
#include "Process.h"

#pragma warning(push, 4)

// Process::Create<ProcessClass::x86>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<ProcessClass::x86>(const std::shared_ptr<VirtualMachine>&, uapi::pid_t pid, const FileSystem::AliasPtr&,
	const FileSystem::AliasPtr&, const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);

#ifdef _M_X64
// Process::Create<ProcessClass::x86_64>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<ProcessClass::x86_64>(const std::shared_ptr<VirtualMachine>&, uapi::pid_t pid, const FileSystem::AliasPtr&,
	const FileSystem::AliasPtr&, const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);
#endif

//-----------------------------------------------------------------------------
// Process Constructor
//
// Arguments:
//
//	TODO: DOCUMENT THEM

Process::Process(ProcessClass _class, std::unique_ptr<Host>&& host, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, 
	std::unique_ptr<TaskState>&& taskstate, 
	std::vector<std::unique_ptr<ProcessSection>>&& sections, void* programbreak) : 
	m_class(_class), m_host(std::move(host)), m_pid(pid), m_rootdir(rootdir), m_workingdir(workingdir), m_taskstate(std::move(taskstate)), m_programbreak(programbreak)
{
	// Insert all of the provided memory sections into the member collection
	for(auto& iterator : sections) m_sections.push_back(std::move(iterator));
}

//-----------------------------------------------------------------------------
// Process::AddHandle
//
// Adds a file system handle to the process
//
// Arguments:
//
//	handle		- Handle instance to be added to the process

int Process::AddHandle(const FileSystem::HandlePtr& handle)
{
	Concurrency::reader_writer_lock::scoped_lock writer(m_handlelock);

	// Allocate a new index (file descriptor) for the handle and insert it
	int index = m_indexpool.Allocate();
	if(m_handles.insert(std::make_pair(index, handle)).second) return index;

	// Insertion failed, release the index back to the pool and throw
	m_indexpool.Release(index);
	throw LinuxException(LINUX_EBADF);
}

//-----------------------------------------------------------------------------
// Process::AddHandle
//
// Adds a file system handle to the process with a specific file descriptor index
//
// Arguments:
//
//	fd			- Specific file descriptor index to use
//	handle		- Handle instance to be added to the process

int Process::AddHandle(int fd, const FileSystem::HandlePtr& handle)
{
	Concurrency::reader_writer_lock::scoped_lock writer(m_handlelock);

	// Attempt to insert the handle using the specified index
	if(m_handles.insert(std::make_pair(fd, handle)).second) return fd;
	throw LinuxException(LINUX_EBADF);
}

//------------------------------------------------------------------------------
// Process::AllocateMemory (private)
//
// Allocates and commits memory in the process address space
//
// Arguments:
//
//	address			- Optional base address
//	length			- Required allocation length
//	protection		- Linux memory protection flags (not Windows flags)

void* Process::AllocateMemory(void* address, size_t length, uint32_t protection)
{
	MEMORY_BASIC_INFORMATION					meminfo;		// Virtual memory information

	// Allocations cannot be zero-length
	if(length == 0) throw LinuxException(LINUX_EINVAL);

	// Prevent changes to the section collection while this operation is taking place
	Concurrency::reader_writer_lock::scoped_lock writer(m_sectionlock);

	// No specific address was requested, let the system decide where the new section should go
	if(address == nullptr) {

		std::unique_ptr<ProcessSection> section = ProcessSection::Create(m_host->ProcessHandle, align::up(length, SystemInformation::AllocationGranularity));
		address = section->Allocate(section->BaseAddress, length, uapi::LinuxProtToWindowsPageFlags(protection));
		m_sections.push_back(std::move(section));
		return address;
	}

	// A specific address was requested, first scan over the process address space and fill in any holes
	// with new sections to ensure a contiguous region
	uintptr_t fillbegin = align::down(uintptr_t(address), SystemInformation::AllocationGranularity);
	uintptr_t fillend = align::up((uintptr_t(address) + length), SystemInformation::AllocationGranularity);

	while(fillbegin < fillend) {

		// Query the information about the virtual memory beginning at the current address
		if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(fillbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
			throw LinuxException(LINUX_EACCES, Win32Exception());

		// If the region is free (MEM_FREE), create a new memory section in the free space
		if(meminfo.State == MEM_FREE) {

			size_t filllength = min(meminfo.RegionSize, align::up(fillend - fillbegin, SystemInformation::AllocationGranularity));
			m_sections.emplace_back(ProcessSection::Create(m_host->ProcessHandle, meminfo.BaseAddress, filllength));
		}

		fillbegin += meminfo.RegionSize;
	}

	// The entire required virtual address space is now available for the allocation operation
	uintptr_t allocbegin = uintptr_t(address);
	uintptr_t allocend = allocbegin + length;

	while(allocbegin < allocend) {

		// Locate the section object that matches the current allocation base address
		const auto& found = std::find_if(m_sections.begin(), m_sections.end(), [&](const std::unique_ptr<ProcessSection>& section) -> bool {
			return ((allocbegin >= uintptr_t(section->BaseAddress)) && (allocbegin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, throw LINUX_ENOMEM
		if(found == m_sections.end()) throw LinuxException(LINUX_ENOMEM, Win32Exception(ERROR_INVALID_ADDRESS));

		// Determine the length of the allocation to request from this section and request it
		size_t alloclen = min((*found)->Length - (allocbegin - uintptr_t((*found)->BaseAddress)), allocend - allocbegin);
		(*found)->Allocate(reinterpret_cast<void*>(allocbegin), alloclen, uapi::LinuxProtToWindowsPageFlags(protection));

		allocbegin += alloclen;
	}

	return address;					// Return the originally requested address
}

//-----------------------------------------------------------------------------
// Process::CheckHostProcessClass<x86> (static, private)
//
// Verifies that the created host process is 32-bit
//
// Arguments:
//
//	process		- Handle to the created host process

template <> inline void Process::CheckHostProcessClass<ProcessClass::x86>(HANDLE process)
{
	BOOL			result;				// Result from IsWow64Process

	// 32-bit systems can only create 32-bit processes; nothing to worry about
	if(SystemInformation::ProcessorArchitecture == SystemInformation::Architecture::Intel) return;

	// 64-bit system; verify that the process is running under WOW64
	if(!IsWow64Process(process, &result)) throw Win32Exception();
	if(!result) throw Exception(E_PROCESSINVALIDX86HOST);
}

//-----------------------------------------------------------------------------
// Process::CheckHostProcessClass<x86_64> (static, private)
//
// Verifies that the created host process is 64-bit
//
// Arguments:
//
//	process		- Handle to the created host process

#ifdef _M_X64
template <> inline void Process::CheckHostProcessClass<ProcessClass::x86_64>(HANDLE process)
{
	BOOL				result;				// Result from IsWow64Process

	// 64-bit system; verify that the process is not running under WOW64
	if(!IsWow64Process(process, &result)) throw Win32Exception();
	if(result) throw Exception(E_PROCESSINVALIDX64HOST);
}
#endif

//-----------------------------------------------------------------------------
// Process::Clone
//
// Clones the running process into a new child process
//
// Arguments:
//
// TODO
std::shared_ptr<Process> Process::Clone(const std::shared_ptr<VirtualMachine>& vm, uint32_t flags, void* taskstate, size_t taskstatelen)
{
	(vm);
	(flags);

	// need parent


	std::unique_ptr<TaskState> ts = TaskState::Create(m_class, taskstate, taskstatelen);

	std::vector<std::unique_ptr<ProcessSection>>	sections;
	std::shared_ptr<Process> child;

	uapi::pid_t pid = 0;

	Suspend();										// Suspend the parent process

	// Create the new host process
	std::unique_ptr<Host> host = Host::Create(vm->GetProperty(VirtualMachine::Properties::HostProcessBinary32).c_str(), 
		vm->GetProperty(VirtualMachine::Properties::HostProcessArguments).c_str(), nullptr, 0);

	try {

		Concurrency::reader_writer_lock::scoped_lock_read reader(m_sectionlock);

		// TESTING: Clone all of the memory sections from the parent process into the child process
		for(auto iterator = m_sections.cbegin(); iterator != m_sections.cend(); iterator++) {
			sections.push_back(ProcessSection::FromSection(*iterator, host->ProcessHandle, ProcessSection::Mode::CopyOnWrite));
			(*iterator)->ChangeMode(ProcessSection::Mode::CopyOnWrite);
		}

		try {
			
			pid = vm->AllocatePID();
			child = std::make_shared<Process>(m_class, std::move(host), pid, m_rootdir, m_workingdir, std::move(ts), std::move(sections), m_programbreak);
		}

		catch(...) { vm->ReleasePID(pid); throw; }

		// TEST: CLONE ALL OF THE HANDLES - this isn't how it needs to be done, just trying it out
		for(auto iterator : m_handles) 
			child->AddHandle(iterator.first, iterator.second);
	}

	// todo: Terminate expects a LINUX error code, not a Windows one!
	catch(std::exception& ex) { host->Terminate(E_FAIL); throw; (ex); /* TODO */ }

	Resume();
	child->Resume();

	return child;
}

//-----------------------------------------------------------------------------
// Process::Create (static)
//
// Constructs a new process instance from an ELF binary
//
// Arguments:
//
//	vm			- Reference to the VirtualMachine instance
//	rootdir		- Initial process root directory alias
//	workingdir	- Initial process working directory alias
//	handle		- Open FileSystem::Handle against the ELF binary to load
//	argv		- ELF command line arguments from caller
//	envp		- ELF environment variables from caller
//	hostpath	- Path to the external host to load
//	hostargs	- Command line arguments to pass to the host

template <ProcessClass _class>
std::shared_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const FileSystem::AliasPtr& rootdir, 
	const FileSystem::AliasPtr& workingdir, const FileSystem::HandlePtr& handle, const uapi::char_t** argv, const uapi::char_t** envp, 
	const tchar_t* hostpath, const tchar_t* hostargs)
{
	using elf = elf_traits<_class>;

	std::unique_ptr<ElfImage>		executable;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;			// Optional interpreter image specified by executable
	uint8_t							random[16];				// 16-bytes of random data for AT_RANDOM auxvec

	// Create the external host process (suspended by default) and verify the class/architecture
	// as this will all go south very quickly if it's not the expected architecture
	// todo: need the handles to stuff that are to be inherited (signals, etc)
	std::unique_ptr<Host> host = Host::Create(hostpath, hostargs, nullptr, 0);
	CheckHostProcessClass<_class>(host->ProcessHandle);

	try {

		// The ELF loader requires the file handle to be at position zero
		handle->Seek(0, LINUX_SEEK_SET);

		// Generate the AT_RANDOM data to be associated with this process
		Random::Generate(random, 16);

		// Attempt to load the binary image into the process, then check for an interpreter
		executable = ElfImage::Load<_class>(handle, host->ProcessHandle);
		if(executable->Interpreter) {

			// Acquire a handle to the interpreter binary and attempt to load that into the process
			bool absolute = (*executable->Interpreter == '/');
			FileSystem::HandlePtr interphandle = vm->OpenExecutable(rootdir, (absolute) ? rootdir : workingdir, executable->Interpreter);
			interpreter = ElfImage::Load<_class>(interphandle, host->ProcessHandle);
		}

		// Construct the ELF arguments stack image for the hosted process
		ElfArguments args(argv, envp);

		(LINUX_AT_EXECFD);																		//  2 - TODO
		if(executable->ProgramHeaders) {

			args.AppendAuxiliaryVector(LINUX_AT_PHDR, executable->ProgramHeaders);				//  3
			args.AppendAuxiliaryVector(LINUX_AT_PHENT, sizeof(typename elf::progheader_t));		//  4
			args.AppendAuxiliaryVector(LINUX_AT_PHNUM, executable->NumProgramHeaders);			//  5
		}

		args.AppendAuxiliaryVector(LINUX_AT_PAGESZ, SystemInformation::PageSize);				//  6
		if(interpreter) args.AppendAuxiliaryVector(LINUX_AT_BASE, interpreter->BaseAddress);	//  7
		args.AppendAuxiliaryVector(LINUX_AT_FLAGS, 0);											//  8
		args.AppendAuxiliaryVector(LINUX_AT_ENTRY, executable->EntryPoint);						//  9
		(LINUX_AT_NOTELF);																		// 10 - NOT IMPLEMENTED
		(LINUX_AT_UID);																			// 11 - TODO
		(LINUX_AT_EUID);																		// 12 - TODO
		(LINUX_AT_GID);																			// 13 - TODO
		(LINUX_AT_EGID);																		// 14 - TODO
		args.AppendAuxiliaryVector(LINUX_AT_PLATFORM, elf::platform);							// 15
		(LINUX_AT_HWCAP);																		// 16 - TODO
		(LINUX_AT_CLKTCK);																		// 17 - TODO
		args.AppendAuxiliaryVector(LINUX_AT_SECURE, 0);											// 23
		(LINUX_AT_BASE_PLATFORM);																// 24 - NOT IMPLEMENTED
		args.AppendAuxiliaryVector(LINUX_AT_RANDOM, random, 16);								// 25
		(LINUX_AT_HWCAP2);																		// 26 - TODO
		(LINUX_AT_EXECFN);																		// 31 - TODO -- WHATEVER WAS PASSED INTO EXECVE() GOES HERE
		(LINUX_AT_SYSINFO);																		// 32 - TODO MAY NOT IMPLEMENT?
		//args.AppendAuxiliaryVector(LINUX_AT_SYSINFO_EHDR, vdso->BaseAddress);					// 33 - TODO NEEDS VDSO

		// Allocate the stack for the process, using the currently set initial stack size for the Virtual Machine
		//// TODO: NEED TO GET STACK LENGTH FROM RESOURCE LIMITS SET FOR VIRTUAL MACHINE
		std::unique_ptr<ProcessSection> stack = ProcessSection::Create(host->ProcessHandle, 1 MiB, ProcessSection::Mode::Private);

		// Commit the entire stack, placing guard pages at both the beginning and end of the section
		stack->Allocate(stack->BaseAddress, stack->Length, PAGE_READWRITE);
		stack->Protect(stack->BaseAddress, SystemInformation::PageSize, PAGE_READONLY | PAGE_GUARD);
		stack->Protect(reinterpret_cast<void*>(uintptr_t(stack->BaseAddress) + stack->Length - SystemInformation::PageSize), SystemInformation::PageSize, PAGE_READONLY | PAGE_GUARD);

		// Write the ELF arguments into the read/write portion of the process stack section and get the resultant pointer
		void* stack_pointer = args.GenerateProcessStack<_class>(host->ProcessHandle, reinterpret_cast<void*>(uintptr_t(stack->BaseAddress) + SystemInformation::PageSize), 
			stack->Length - (SystemInformation::PageSize * 2));

		// Load the remainder of the StartupInfo structure with the necessary information to get the ELF binary running
		const void* entry_point = (interpreter) ? interpreter->EntryPoint : executable->EntryPoint;
		void* program_break = executable->ProgramBreak;

		// TESTING NEW STARTUP INFORMATION
		std::unique_ptr<TaskState> ts = TaskState::Create(_class, entry_point, stack_pointer);

		// The allocated ProcessSection instances need to be transferred to the Process instance
		std::vector<std::unique_ptr<ProcessSection>> sections;
		sections.push_back(std::move(executable));
		sections.push_back(std::move(stack));
		if(interpreter) sections.push_back(std::move(interpreter));

		// Create the Process object, transferring the host, startup information and allocated memory sections
		return std::make_shared<Process>(_class, std::move(host), pid, rootdir, workingdir, std::move(ts), std::move(sections), program_break);
	}

	// Terminate the host process on exception since it doesn't get killed by the Host destructor
	// TODO: should be LinuxException wrapping the underlying one?  Only case right now where
	// that wouldn't be true is interpreter's OpenExec() call which should already throw LinuxExceptions
	// TODO: exit code should be a LINUX exit code, not a Windows one
	catch(...) { host->Terminate(E_FAIL); throw; }
}

//-----------------------------------------------------------------------------
// Process::GetHandle
//
// Accesses a file system handle referenced by the process
//
// Arguments:
//
//	index		- Index (file descriptor) of the target handle

FileSystem::HandlePtr Process::GetHandle(int index)
{
	Concurrency::reader_writer_lock::scoped_lock_read reader(m_handlelock);

	try { return m_handles.at(index); }
	catch(...) { throw LinuxException(LINUX_EBADF); }
}

//-----------------------------------------------------------------------------
// Process::GetInitialTaskState
//
// Gets a copy of the original task state information for the process, the
// contents of which varies based on if the process is 32 or 64-bit
//
// Arguments:
//
//	taskstate	- Pointer to receive the task state information
//	length		- Length of the startup information buffer

void Process::GetInitialTaskState(void* startinfo, size_t length)
{
	return m_taskstate->CopyTo(startinfo, length);
}

//-----------------------------------------------------------------------------
// Process::MapMemory
//
// Creates a memory mapping for the process
//
// Arguments:
//
//	address		- Optional fixed address to use for the mapping
//	length		- Length of the requested mapping
//	prot		- Memory protection flags
//	flags		- Memory mapping flags, must include MAP_PRIVATE
//	fd			- Optional file descriptor from which to map
//	offset		- Optional offset into fd from which to map

void* Process::MapMemory(void* address, size_t length, int prot, int flags, int fd, uapi::loff_t offset)
{
	FileSystem::HandlePtr		handle = nullptr;		// Handle to the file object

	_ASSERTE(m_host);
	_ASSERTE(m_host->ProcessHandle);

	// MAP_HUGETLB is not currently supported, but may be possible in the future
	if(flags & LINUX_MAP_HUGETLB) throw LinuxException(LINUX_EINVAL);

	// MAP_GROWSDOWN is not supported
	if(flags & LINUX_MAP_GROWSDOWN) throw LinuxException(LINUX_EINVAL);

	// Suggested base addresses are not supported, switch the address to NULL if MAP_FIXED is not set
	if((flags & LINUX_MAP_FIXED) == 0) address = nullptr;

	// Non-anonymous mappings require a valid file descriptor to be specified
	if(((flags & LINUX_MAP_ANONYMOUS) == 0) && (fd <= 0)) throw LinuxException(LINUX_EBADF);
	if(fd > 0) handle = GetHandle(fd)->Duplicate(LINUX_O_RDONLY);

	// Attempt to allocate the process memory and adjust address to that base if it was NULL
	void* base = AllocateMemory(address, length, prot);
	if(address == nullptr) address = base;

	// If a file handle was specified, copy data from the file into the allocated region,
	// private mappings need not be concerned with writing this data back to the file
	if(handle != nullptr) {

		uintptr_t	dest = uintptr_t(address);			// Easier pointer math as uintptr_t
		size_t		read;								// Bytes read from the source file
		SIZE_T		written;							// Result from NtWriteVirtualMemory

		// TODO: Both tempfs and hostfs should be able to handle memory mappings now with
		// the introduction of sections; this would be much more efficient that way.  Such
		// a call would need to fail for FS that can't support it, and fall back to this
		// type of implementation.  Also evaluate the same in ElfImage, mapping the source
		// file would be better than having to read it from a stream

		HeapBuffer<uint8_t> buffer(SystemInformation::AllocationGranularity);	// 64K buffer

		// Seek the file handle to the specified offset
		if(handle->Seek(offset, LINUX_SEEK_SET) != offset) throw LinuxException(LINUX_EINVAL);

		do {
			
			// Copy the next chunk of bytes from the source file into the process' memory space
			read = handle->Read(buffer, min(length, buffer.Size));
			NTSTATUS result = NtApi::NtWriteVirtualMemory(m_host->ProcessHandle, reinterpret_cast<void*>(dest), buffer, read, &written);
			if(result != NtApi::STATUS_SUCCESS) throw LinuxException(LINUX_EACCES, StructuredException(result));

			dest += written;						// Increment destination pointer
			length -= read;							// Decrement remaining bytes to be copied

		} while((length > 0) && (read > 0));
	};

	return address;
}

//-----------------------------------------------------------------------------
// Process::getParentProcessId

uapi::pid_t Process::getParentProcessId(void) const
{
	// If the parent process is still alive, return that identifier, otherwise
	// the parent process becomes the init process (id 1)
	std::shared_ptr<Process> parent = m_parent.lock();
	return (parent) ? parent->ProcessId : VirtualMachine::PROCESSID_INIT;
}

//-----------------------------------------------------------------------------
// Process::ProtectMemory
//
// Assigns protection flags for an allocated region of memory
//
// Arguments:
//
//	address		- Base address of the region to be protected
//	length		- Length of the region to be protected
//	prot		- New protection flags for the memory region 

void Process::ProtectMemory(void* address, size_t length, int protect)
{
	MEMORY_BASIC_INFORMATION	meminfo;			// Virtual memory information
	DWORD						oldprotection;		// Previously set protection flags

	_ASSERTE(m_host);
	_ASSERTE(m_host->ProcessHandle != INVALID_HANDLE_VALUE);

	// Determine the starting and ending points for the operation; Windows will automatically
	// adjust these values to align on the proper page size during VirtualProtect
	uintptr_t begin = uintptr_t(address);
	uintptr_t end = begin + length;

	// Scan the requested memory region and set the protection flags
	while(begin < end) {

		// Query the information about the current region to be protected
		if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
			throw LinuxException(LINUX_EACCES, Win32Exception());

		// Attempt to assign the protection flags for the current region of memory
		if(!VirtualProtectEx(m_host->ProcessHandle, reinterpret_cast<void*>(begin), min(end - begin, meminfo.RegionSize), 
			uapi::LinuxProtToWindowsPageFlags(protect), &oldprotection)) throw LinuxException(LINUX_ENOMEM, Win32Exception());

		begin += meminfo.RegionSize;		// Move to the next region
	}
}

//-----------------------------------------------------------------------------
// Process::ReadMemory
//
// Reads data from the client process into a local buffer.  If a fault will 
// occur or is trapped, the read operation will stop prior to requested length
//
// Arguments:
//
//	address		- Address in the client process from which to read
//	buffer		- Local output buffer
//	length		- Size of the local output buffer, maximum bytes to read

size_t Process::ReadMemory(const void* address, void* buffer, size_t length)
{
	MEMORY_BASIC_INFORMATION	meminfo;		// Virtual memory information
	SIZE_T						read;			// Number of bytes read from process

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Query the status of the memory at the specified address
	if(!VirtualQueryEx(m_host->ProcessHandle, address, &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
		throw LinuxException(LINUX_EACCES, Win32Exception());
	
	// Ensure the the memory has been committed
	if(meminfo.State != MEM_COMMIT) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// TODO: WHY AM I MAKING IT THIS COMPLICATED; LET THE FUNCTION CALL FAIL

	// Check the protection status of the memory region, it cannot be NOACCESS or EXECUTE to continue
	DWORD protection = meminfo.Protect & 0xFF;
	if((protection == PAGE_NOACCESS) || (protection == PAGE_EXECUTE)) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// Attempt to read up to the requested length or to the end of the region, whichever is smaller
	NTSTATUS result = NtApi::NtReadVirtualMemory(m_host->ProcessHandle, address, buffer, min(length, meminfo.RegionSize), &read);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException(LINUX_EFAULT, StructuredException(result));

	return read;
}

//-----------------------------------------------------------------------------
// Process::ReleaseMemory (private)
//
// Releases memory from the process address space
//
// Arguments:
//
//	address		- Base address of the memory region to be released
//	length		- Length of the memory region to be released

void Process::ReleaseMemory(void* address, size_t length)
{
	uintptr_t begin = uintptr_t(address);			// Start address as uintptr_t
	uintptr_t end = begin + length;					// End address as uintptr_t

	// Prevent changes to the section collection while this operation is taking place
	Concurrency::reader_writer_lock::scoped_lock writer(m_sectionlock);

	while(begin < end) {

		// Locate the section object that matches the current base address
		const auto& found = std::find_if(m_sections.begin(), m_sections.end(), [&](const std::unique_ptr<ProcessSection>& section) -> bool {
			return ((begin >= uintptr_t(section->BaseAddress)) && (begin < (uintptr_t(section->BaseAddress) + section->Length)));
		});

		// No matching section object exists, treat this as a no-op -- don't throw an exception
		if(found == m_sections.end()) return;

		// Determine how much to release from this section and release it
		size_t freelength = min((*found)->Length - (begin - uintptr_t((*found)->BaseAddress)), end - begin);
		(*found)->Release(reinterpret_cast<void*>(begin), freelength);

		// If the section is empty after the release, remove it from the process
		if((*found)->Empty) m_sections.erase(found);

		begin += freelength;
	}
}

//-----------------------------------------------------------------------------
// Process::Resume
//
// Resumes the process from a suspended state
//
// Arguments:
//
//	NONE

void Process::Resume(void)
{
	NTSTATUS result = NtApi::NtResumeProcess(m_host->ProcessHandle);
	if(result != 0) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// Process::RemoveHandle
//
// Removes a file system handle from the process
//
// Arguments:
//
//	index		- Index (file descriptor) of the target handle

void Process::RemoveHandle(int index)
{
	Concurrency::reader_writer_lock::scoped_lock writer(m_handlelock);

	// Remove the index from the handle collection and return it to the pool
	if(m_handles.erase(index) == 0) throw LinuxException(LINUX_EBADF);
	m_indexpool.Release(index);
}

//-----------------------------------------------------------------------------
// Process::SetProgramBreak
//
// Adjusts the program break address by increasing or decreasing the number
// of allocated pages immediately following the loaded binary image
//
// Arguments:
//
//	address		- Requested program break address

void* Process::SetProgramBreak(void* address)
{
	// NULL can be passed in as the address to retrieve the current program break
	if(address == nullptr) return m_programbreak;

	// The real break address must be aligned to a page boundary
	void* oldbreak = align::up(m_programbreak, SystemInformation::PageSize);
	void* newbreak = align::up(address, SystemInformation::PageSize);

	// If the aligned addresses are not the same, the actual break must be changed
	if(oldbreak != newbreak) {

		// Calculate the delta between the current and requested address
		intptr_t delta = intptr_t(newbreak) - intptr_t(oldbreak);

		try {

			// Allocate or release program break address space based on the calculated delta.
			if(delta > 0) AllocateMemory(oldbreak, delta, LINUX_PROT_WRITE | LINUX_PROT_READ);
			else ReleaseMemory(newbreak, delta);
		}

		// Return the previously set program break address if it could not be adjusted,
		// this operation is not intended to return any error codes
		catch(...) { return m_programbreak; }
	}

	// Store and return the requested address, not the page-aligned address
	m_programbreak = address;
	return m_programbreak;
}

//-----------------------------------------------------------------------------
// Process::Spawn (static)
//
// Creates a new process
//
// Arguments:
//
//	vm			- VirtualMachine instance
//	pid			- Optional PID to assign to the new process or -1
//	filename	- Path to the executable to create the process from
//	argv		- Arguments to pass to the new process
//	envp		- Environment for the new process

std::shared_ptr<Process> Process::Spawn(const std::shared_ptr<VirtualMachine>& vm, uapi::pid_t pid, const uapi::char_t* filename,
	const uapi::char_t** argv, const uapi::char_t** envp)
{
	// When creating a process directly via this function, the root and working directory are the filesystem root
	std::shared_ptr<FileSystem> rootfs = vm->RootFileSystem;

	// Attempt to open a handle to the specified executable, relative to the file system root node
	std::shared_ptr<FileSystem::Handle> handle = vm->OpenExecutable(rootfs->Root, rootfs->Root, filename);


	return nullptr;
}

//-----------------------------------------------------------------------------
// Process::Suspend
//
// Suspends the process
//
// Arguments:
//
//	NONE

void Process::Suspend(void)
{
	NTSTATUS result = NtApi::NtSuspendProcess(m_host->ProcessHandle);
	if(result != 0) throw StructuredException(result);
}

//-----------------------------------------------------------------------------
// Process::UnmapMemory
//
// Releases a memory region allocated with MapMemory
//
// Arguments:
//
//	address		- Base address of the region to be released
//	length		- Length of the region to be released

void Process::UnmapMemory(void* address, size_t length)
{
	ReleaseMemory(address, length);
}

size_t Process::WriteMemory(void* address, const void* buffer, size_t length)
{
	SIZE_T					written;			// Number of bytes written

	_ASSERTE(buffer);
	if((buffer == nullptr) || (length == 0)) return 0;

	// Attempt to write the data to the target process
	NTSTATUS result = NtApi::NtWriteVirtualMemory(m_host->ProcessHandle, address, buffer, length, &written);
	if(result != NtApi::STATUS_SUCCESS) throw LinuxException(LINUX_EFAULT, StructuredException(result));

	return written;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
