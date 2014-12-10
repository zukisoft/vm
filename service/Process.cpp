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
#include "Process.h"

#pragma warning(push, 4)

// Process::Create<ElfClass::x86>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<ElfClass::x86>(const std::shared_ptr<VirtualMachine>&, const FileSystem::AliasPtr&,
	const FileSystem::AliasPtr&, const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);

#ifdef _M_X64
// Process::Create<ElfClass::x86_64>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<ElfClass::x86_64>(const std::shared_ptr<VirtualMachine>&, const FileSystem::AliasPtr&,
	const FileSystem::AliasPtr&, const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);
#endif

//-----------------------------------------------------------------------------
// Process Constructor
//
// Arguments:
//
//	TODO: DOCUMENT THEM

Process::Process(std::unique_ptr<Host>&& host, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir, StartupInfo&& startinfo, std::vector<std::unique_ptr<MemorySection>>&& sections) : 
	m_host(std::move(host)), m_rootdir(rootdir), m_workingdir(workingdir), m_startinfo(startinfo), m_sections(std::move(sections)) 
{
	_ASSERTE(m_startinfo.ProgramBreak);

	// Insert a guard page at the provided program break address to prevent heap
	// underruns from trampling the loaded binary image sections.  This also prevents
	// any need to keep track of what the original break address happened ot be
	AllocateMemory(m_startinfo.ProgramBreak, SystemInformation::PageSize, PAGE_READONLY | PAGE_GUARD);
	m_break = reinterpret_cast<void*>(uintptr_t(m_startinfo.ProgramBreak) + SystemInformation::PageSize);
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
//	address		- Optional base address
//	length		- Required allocation length
//	protection	- WINDOWS memory protection flags (not LINUX_XXXX)

void* Process::AllocateMemory(void* address, size_t length, uint32_t protection)
{
	// Allocations cannot be zero-length
	if(length == 0) throw LinuxException(LINUX_EINVAL);

	// If no specific address was requested, let the system decide where to put the new 
	// section of memory, but ensure that the length is aligned up to the system allocation
	// granularity to prevent address space holes from forming
	if(address == nullptr) {

		// Reserve an anonymous memory section and save the base address
		std::unique_ptr<MemorySection> anonymous = MemorySection::Reserve(m_host->ProcessHandle, length, SystemInformation::AllocationGranularity);
		void* base = anonymous->BaseAddress;

		// Commit up to the requested length of pages within the new anonymous section
		anonymous->Commit(base, length, protection);

		// Insert the section into the member collection and return the generated base address
		m_sections.push_back(std::move(anonymous));
		return base;
	}

	// A specific virtual address was requested, scan over the existing virtual memory
	// and ensure that it's either all reserved or all free.  Fill in any free areas
	// with new memory sections, and commit everything using the specified protection

	std::vector<std::unique_ptr<MemorySection>>	sections;		// Newly reserved sections
	MEMORY_BASIC_INFORMATION					meminfo;		// Virtual memory information
	
	// First iterate over the entire aligned memory area and fill in free regions with new sections
	uintptr_t allocbegin = align::down(uintptr_t(address), SystemInformation::AllocationGranularity);
	uintptr_t allocend = align::up((uintptr_t(address) + length), SystemInformation::AllocationGranularity);

	while(allocbegin < allocend) {

		// Query the information about the section beginning at the current address
		if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(allocbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
			throw LinuxException(LINUX_EACCES, Win32Exception());

		// If the region is free (MEM_FREE), attempt to create a section in the free space, ensuring that we get
		// a block of addresses aligned to the allocation granularity to avoid address space holes
		if(meminfo.State == MEM_FREE) {

			sections.push_back(MemorySection::Reserve(m_host->ProcessHandle, meminfo.BaseAddress, 
				min(meminfo.RegionSize, (allocend - allocbegin)), SystemInformation::AllocationGranularity));
		}

		allocbegin += meminfo.RegionSize;
	}

	// Now iterate over the range of requested pages, which should all now be reserved (MEM_RESERVE) and
	// commit them with the specified protection flags
	uintptr_t commitbegin = align::down(uintptr_t(address), SystemInformation::PageSize);
	uintptr_t commitend = align::up(commitbegin + length, SystemInformation::PageSize);

	while(commitbegin < commitend) {

		// Query the information about the section beginning at the current address
		if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(commitbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
			throw LinuxException(LINUX_EACCES, Win32Exception());

		// The page(s) must be reserved at this point, if they are already committed that's a problem
		if(meminfo.State != MEM_RESERVE) throw LinuxException(LINUX_ENOMEM, Win32Exception(ERROR_INVALID_ADDRESS));

		// Commit the memory with the requested protection flags
		if(VirtualAllocEx(m_host->ProcessHandle, meminfo.BaseAddress, min(meminfo.RegionSize, commitend - commitbegin), MEM_COMMIT, 
			protection) == nullptr) throw Win32Exception();

		commitbegin += meminfo.RegionSize;
	}

	// Add any sections generated above to the member collection
	for(auto& iterator : sections) { m_sections.push_back(std::move(iterator)); }

	// Return the originally requested virtual address
	return address;
}

//-----------------------------------------------------------------------------
// Process::CheckHostProcessClass<x86> (static, private)
//
// Verifies that the created host process is 32-bit
//
// Arguments:
//
//	process		- Handle to the created host process

template <> inline void Process::CheckHostProcessClass<ElfClass::x86>(HANDLE process)
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
template <> inline void Process::CheckHostProcessClass<ElfClass::x86_64>(HANDLE process)
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

std::shared_ptr<Process> Process::Clone(const std::shared_ptr<VirtualMachine>& vm, const tchar_t* hostpath, const tchar_t* hostargs, uint32_t flags)
{
	(vm);
	(hostpath);
	(hostargs);
	(flags);

	// general thoughts
	//
	// SUSPEND PARENT PROCESS
	// CREATE NEW HOST FOR CHILD (SUSPENDED)
	// CLONE ALL EXISTING SECTIONS FROM PARENT TO CHILD USING NEW SECTION CLASS
	// RESUME PARENT PROCESS
	// RESUME CHILD PROCESS
	// CHILD REGISTERS ITSELF WITH sys32_acquire_context
	// CHILD CREATES THREAD ZERO SUSPENDED
	// PUSH PARENT CONTEXT TO CHILD THREAD ZERO WITH MAGIC
	// CHILD THREAD ZERO PICKS UP WHERE PARENT LEFT OFF
	// EVERYTHING IS AWESOME

	Suspend();										// Suspend the parent process

	// Create the new host process
	// TODO: WILL NEED NEW ARGUMENTS -- IT WILL NEED TO KNOW IT'S A CLONE
	std::unique_ptr<Host> host = Host::Create(hostpath, hostargs, nullptr, 0);

	//try {
	//	// LOCK SECTIONS COLLECTION HERE
	//
	//	section_set_t newsections;

	//	// Clone all of the sections from the parent process into the child process
	//	for(auto iterator = m_sections.cbegin(); iterator != m_sections.cend(); iterator++)
	//		newsections.insert((*iterator)->Clone(host->ProcessHandle, MemorySection::CloneMode::SharedCopyOnWrite));

	//	int x = 123;
	//	(x);
	//	// UNLOCK SECTIONS COLLECTION HERE
	//}
	//catch(...) { host->Terminate(E_FAIL); throw; }

	// TEST TO ACQUIRE CONTEXT; NOT SURE IF THIS WILL BE USED
	//DWORD x = 0;  // get from OutputDebugString for process and type into debugger for now
	//HANDLE h = OpenThread(THREAD_ALL_ACCESS, FALSE, x);
	//zero_init<CONTEXT> c;
	//c.ContextFlags = CONTEXT_ALL;
	//BOOL result = GetThreadContext(h, &c);		// Wow64GetThreadContext if _M_X64 and 32-bit parent
	//DWORD dw = GetLastError();
	//if(h != nullptr) CloseHandle(h);

	// Just kill it for now
	host->Terminate(E_FAIL);

	Resume();										// Resume the parent process

	return nullptr;
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

template <ElfClass _class>
std::shared_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::AliasPtr& rootdir, const FileSystem::AliasPtr& workingdir,
	const FileSystem::HandlePtr& handle, const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs)
{
	using elf = elf_traits<_class>;

	std::unique_ptr<ElfImage>		executable;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;			// Optional interpreter image specified by executable
	uint8_t							random[16];				// 16-bytes of random data for AT_RANDOM auxvec
	StartupInfo						startinfo;				// Hosted process startup information

	std::vector<std::unique_ptr<MemorySection>> sections;

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

		// Generate the stack image for the process in it's address space
		ElfArguments::StackImage stackimg = args.GenerateStackImage<_class>(host->ProcessHandle);

		// Load the StartupInfo structure with the necessary information to get the ELF binary running
		startinfo.EntryPoint = (interpreter) ? interpreter->EntryPoint : executable->EntryPoint;
		startinfo.ProgramBreak = executable->ProgramBreak;
		startinfo.StackImage = stackimg.BaseAddress;
		startinfo.StackImageLength = stackimg.Length;

		////////////////
		if(executable->Interpreter) { /* TODO: PUSH_BACK INTERPRETER */ }
		sections.push_back(std::move(executable));
		//////////////////

		// Create the Process object, transferring the host and startup information
		return std::make_shared<Process>(std::move(host), rootdir, workingdir, std::move(startinfo), std::move(sections));
	}

	// Terminate the host process on exception since it doesn't get killed by the Host destructor
	// TODO: should be LinuxException wrapping the underlying one?  Only case right now where
	// that wouldn't be true is interpreter's OpenExec() call which should already throw LinuxExceptions
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
	try { return m_handles.at(index); }
	catch(...) { throw LinuxException(LINUX_EBADF); }
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
	void* base = AllocateMemory(address, length, uapi::LinuxProtToWindowsPageFlags(prot));
	if(address == nullptr) address = base;

	// If a file handle was specified, copy data from the file into the allocated region,
	// private mappings need not be concerned with writing this data back to the file
	if(handle != nullptr) {

		uintptr_t	dest = uintptr_t(address);			// Easier pointer math as uintptr_t
		size_t		read;								// Bytes read from the source file
		SIZE_T		written;							// Result from WriteProcessMemory

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
			if(!WriteProcessMemory(m_host->ProcessHandle, reinterpret_cast<void*>(dest), buffer, read, &written)) 
				throw LinuxException(LINUX_EACCES, Win32Exception());

			dest += written;						// Increment destination pointer
			length -= read;							// Decrement remaining bytes to be copied

		} while((length > 0) && (read > 0));
	};

	return address;
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

void Process::ProtectMemory(void* address, size_t length, int prot)
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
			uapi::LinuxProtToWindowsPageFlags(prot), &oldprotection)) throw LinuxException(LINUX_ENOMEM, Win32Exception());

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

	// Check the protection status of the memory region, it cannot be NOACCESS or EXECUTE to continue
	DWORD protection = meminfo.Protect & 0xFF;
	if((protection == PAGE_NOACCESS) || (protection == PAGE_EXECUTE)) throw LinuxException(LINUX_EFAULT, Exception(E_POINTER));

	// Attempt to read up to the requested length or to the end of the region, whichever is smaller
	if(!ReadProcessMemory(m_host->ProcessHandle, address, buffer, min(length, meminfo.RegionSize), &read))
		throw LinuxException(LINUX_EFAULT, Win32Exception());

	return read;
}

//-----------------------------------------------------------------------------
// Process::ReleaseMemory (private)
//
// Decommits and releases memory from the process address space
//
// Arguments:
//
//	address		- Base address of the memory region to be released
//	length		- Length of the memory region to be released

void Process::ReleaseMemory(void* address, size_t length)
{
	(address);
	(length);
	_RPTF0(_CRT_ASSERT, "Process::ReleaseMemory -- not implemented yet");

	// how will this work?
	//auto iterator = m_test.find(nullptr);

	// This will have to scan the existing sections to see if an entire one can
	// be deleted, otherwise just decommit partial sections and leave in MEM_RESERVE

	// OLD CODE FROM UNMAPMEMORY():

	//MEMORY_BASIC_INFORMATION	meminfo;			// Virtual memory information

	//_ASSERTE(m_host);
	//_ASSERTE(m_host->ProcessHandle != INVALID_HANDLE_VALUE);

	//// Determine the starting and ending points for the operation; Windows will automatically
	//// adjust these values to align on the proper page size during VirtualProtect
	//uintptr_t begin = uintptr_t(address);
	//uintptr_t end = begin + length;

	//// Scan the requested memory region and decommit/release the memory as is appropriate
	//while(begin < end) {

	//	// Query the information about the current region to be released
	//	if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
	//		throw LinuxException(LINUX_EACCES, Win32Exception());

	//	// Don't try to decommit pages that aren't already committed, munmap() should not
	//	// return an error if the page(s) aren't committed (MEM_FREE or MEM_RESERVE)
	//	if(meminfo.State == MEM_COMMIT) {

	//		//
	//		// TODO: Write-back for shared memory maps goes here, need to check if it's
	//		// dirty and the region flags indicate a write-back is necessary.  Determining
	//		// if it's shared can't be done with MEMORY_BASIC_INFORMATION since the current
	//		// intention is to do it all manually via VirtualAllocEx()
	//		//
	//		
	//		if(!VirtualFreeEx(m_host->ProcessHandle, reinterpret_cast<void*>(begin), min(end - begin, meminfo.RegionSize), MEM_DECOMMIT))
	//			throw LinuxException(LINUX_EINVAL, Win32Exception());
	//	}

	//	// If the current base address matches the allocation base address, this region can possibly be released
	//	if(meminfo.BaseAddress == meminfo.AllocationBase) {

	//		// Get updated information about the decommitted region
	//		MEMORY_BASIC_INFORMATION freeinfo;
	//		if(VirtualQueryEx(m_host->ProcessHandle, meminfo.AllocationBase, &freeinfo, sizeof(MEMORY_BASIC_INFORMATION))) {

	//			_ASSERTE(freeinfo.State != MEM_COMMIT);				// Should never happen

	//			// Don't attempt to release a region that is already MEM_FREE
	//			if(freeinfo.State == MEM_RESERVE) {

	//				// Find the original length of the allocation, and if the entire region is now decommitted, release it
	//				auto it = m_mappings.find(reinterpret_cast<void*>(begin));
	//				if((it != m_mappings.end()) && (it->second == freeinfo.RegionSize)) {

	//					if(!VirtualFreeEx(m_host->ProcessHandle, freeinfo.AllocationBase, 0, MEM_RELEASE)) throw LinuxException(LINUX_EINVAL, Win32Exception());
	//					m_mappings.unsafe_erase(it);
	//				}
	//			}
	//		}
	//	}

	//	begin += meminfo.RegionSize;		// Move to the next region
	//}
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
	// unsafe_erase *should* be OK to use here since values are shared_ptrs,
	// but if problems start to happen, this is a good place to look at
	if(m_handles.unsafe_erase(index) == 0) throw LinuxException(LINUX_EBADF);
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
	if(address == nullptr) return m_break;

	// The real break address must be aligned to a page boundary
	void* oldbreak = align::up(m_break, SystemInformation::PageSize);
	void* newbreak = align::up(address, SystemInformation::PageSize);

	// If the aligned addresses are not the same, the actual break must be changed
	if(oldbreak != newbreak) {

		// Calculate the delta between the current and requested address
		intptr_t delta = intptr_t(newbreak) - intptr_t(oldbreak);

		try {

			// Allocate or release program break address space based on the calculated delta.
			// ReleaseMemory will run into the guard page installed in the constructor if the
			// process attempts to underrun the original break address
			if(delta > 0) AllocateMemory(oldbreak, delta, PAGE_READWRITE);
			else ReleaseMemory(newbreak, delta);
		}

		// Return the previously set program break address if it could not be adjusted,
		// this operation is not intended to return any error codes
		catch(...) { return m_break; }
	}

	// Store and return the requested address, not the page-aligned address
	m_break = address;
	return m_break;
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

//-----------------------------------------------------------------------------

#pragma warning(pop)
