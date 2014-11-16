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

// Process::s_sysinfo
//
// Static SYSTEM_INFO information
Process::SystemInfo Process::s_sysinfo;

// Process::Create<ElfClass::x86>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<ElfClass::x86>(const std::shared_ptr<VirtualMachine>&, 
	const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);

#ifdef _M_X64
// Process::Create<ElfClass::x86_64>
//
// Explicit Instantiation of template function
template std::shared_ptr<Process> Process::Create<ElfClass::x86_64>(const std::shared_ptr<VirtualMachine>&, 
	const FileSystem::HandlePtr&, const uapi::char_t**, const uapi::char_t**, const tchar_t*, const tchar_t*);
#endif

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
// Process::AllocateRegion
//
// Allocates a region of memory in the hosted process
//
// Arguments:
//
//	length		- Length of the region to be allocated
//	prot		- Memory protection flags for the memory region
//	flags		- Memory allocation behavioral flags

void* Process::AllocateRegion(size_t length, int prot, int flags)
{
	UNREFERENCED_PARAMETER(flags);

	_ASSERTE(m_host);
	_ASSERTE(m_host->ProcessHandle != INVALID_HANDLE_VALUE);

	// Allocations cannot be zero-length
	if(length == 0) throw LinuxException(LINUX_EINVAL);

	// Attempt to reserve the region of memory from the hosted process aligned up to the
	// system allocation granularity to prevent unusable holes in the address space
	size_t regionlength = align::up(length, MemoryRegion::AllocationGranularity);
	void* region = VirtualAllocEx(m_host->ProcessHandle, nullptr, regionlength, MEM_RESERVE, PAGE_NOACCESS);
	if(!region) throw LinuxException(LINUX_EINVAL, Win32Exception());

	// Attempt to commit and assign protection flags to the allocated region
	if(!VirtualAllocEx(m_host->ProcessHandle, region, length, MEM_COMMIT, uapi::LinuxProtToWindowsPageFlags(prot))) {

		Win32Exception exception;										// Save the Windows exception
		VirtualFreeEx(m_host->ProcessHandle, region, 0, MEM_RELEASE);	// Release the reservation
		throw LinuxException(LINUX_EINVAL, exception);					// Allocation operation failed
	}

	// Keep track of the base address and original allocation size for this mapping
	m_mappings.insert(std::make_pair(region, regionlength));
	return region;
}

//-----------------------------------------------------------------------------
// Process::AllocateFixedRegion
//
// Allocates a region of memory in the hosted process at a specific address;
// makes an assumption that the caller knows what they are doing
//
// Arguments:
//
//	address		- Address at which to allocate the memory region
//	length		- Length of the region to be allocated
//	prot		- Memory protection flags for the memory region
//	flags		- Memory allocation behavioral flags

void* Process::AllocateFixedRegion(void* address, size_t length, int prot, int flags)
{
	MEMORY_BASIC_INFORMATION		meminfo;		// Virtual memory information

	UNREFERENCED_PARAMETER(flags);

	_ASSERTE(m_host);
	_ASSERTE(m_host->ProcessHandle != INVALID_HANDLE_VALUE);

	// Fixed allocations cannot start at zero or be zero-length
	if(address == nullptr) throw LinuxException(LINUX_EINVAL);
	if(length == 0) throw LinuxException(LINUX_EINVAL);

	// Determine the starting and ending points for the reservation
	uintptr_t regionbegin = align::down(uintptr_t(address), MemoryRegion::AllocationGranularity);
	uintptr_t regionend = regionbegin + align::up(length, MemoryRegion::AllocationGranularity);

	// Scan the calculated region to ensure that all required pages are reserved
	while(regionbegin < regionend) {

		// Query the information about the region beginning at the current address
		if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(regionbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
			throw LinuxException(LINUX_EACCES, Win32Exception());

		// If the region is not reserved (MEM_FREE), attempt to reserve it.  This will fail if the region
		// is not aligned to the allocation granularity, which indicates a hole in the address space
		if(meminfo.State == MEM_FREE) {

			size_t regionlength = min(regionend - regionbegin, meminfo.RegionSize);
			if(!VirtualAllocEx(m_host->ProcessHandle, reinterpret_cast<void*>(regionbegin), regionlength,  MEM_RESERVE, PAGE_NOACCESS)) 
				throw LinuxException(LINUX_ENOMEM, Win32Exception());

			// Keep track of the reservation base address and original allocation length
			m_mappings.insert(std::make_pair(reinterpret_cast<void*>(regionbegin), regionlength));
		}

		regionbegin += meminfo.RegionSize;			// Move to the next region
	}

	// Determine the starting and ending points for the commit; Windows will automatically
	// adjust these values to align on the proper page size during VirtualAllocEx
	uintptr_t commitbegin = uintptr_t(address);
	uintptr_t commitend = commitbegin + length;

	// Scan the requested mapping region and (re)commit all pages to memory
	while(commitbegin < commitend) {

		// Query the information about the current region to be committed
		if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(commitbegin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
			throw LinuxException(LINUX_EACCES, Win32Exception());

		switch(meminfo.State) {

			// MEM_COMMIT -- The region has already been committed; de-commit it to allow it to be reset
			//
			case MEM_COMMIT:
				if(!VirtualFreeEx(m_host->ProcessHandle, reinterpret_cast<void*>(commitbegin), min(commitend - commitbegin, meminfo.RegionSize), 
					MEM_DECOMMIT)) throw LinuxException(LINUX_EACCES, Win32Exception());
				// fall through
			
			// MEM_RESERVE -- The region is reserved but has not yet been committed to memory
			// 
			case MEM_RESERVE:
				if(!VirtualAllocEx(m_host->ProcessHandle, reinterpret_cast<void*>(commitbegin), min(commitend - commitbegin, meminfo.RegionSize), 
					MEM_COMMIT, uapi::LinuxProtToWindowsPageFlags(prot))) throw LinuxException(LINUX_EINVAL, Win32Exception());
				break;

			// MEM_FREE -- The region was not properly reserved above or was released by another thread in the process
			//
			default: throw LinuxException(LINUX_EACCES);
		}

		commitbegin += meminfo.RegionSize;
	}

	return address;
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

	// Attempt to reserve and commit the region of memory using the appropriate helper
	address = (address == nullptr) ? AllocateRegion(length, prot, flags) : AllocateFixedRegion(address, length, prot, flags);

	// If a file handle was specified, copy data from the file into the allocated region,
	// private mappings need not be concerned with writing this data back to the file
	if(handle != nullptr) {

		uintptr_t	dest = uintptr_t(address);			// Easier pointer math as uintptr_t
		size_t		read;								// Bytes read from the source file
		SIZE_T		written;							// Result from WriteProcessMemory

		HeapBuffer<uint8_t> buffer(MemoryRegion::AllocationGranularity);	// 64K buffer

		// Seek the file handle to the specified offset
		if(static_cast<size_t>(handle->Seek(offset, LINUX_SEEK_SET)) != offset) throw LinuxException(LINUX_EINVAL);

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

		// Attempt to assign the protection flags for the current region
		if(!VirtualProtectEx(m_host->ProcessHandle, reinterpret_cast<void*>(begin), min(end - begin, meminfo.RegionSize), 
			uapi::LinuxProtToWindowsPageFlags(prot), &oldprotection)) throw LinuxException(LINUX_ENOMEM, Win32Exception());

		begin += meminfo.RegionSize;		// Move to the next region
	}
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
	MEMORY_BASIC_INFORMATION	meminfo;			// Virtual memory information

	_ASSERTE(m_host);
	_ASSERTE(m_host->ProcessHandle != INVALID_HANDLE_VALUE);

	// Determine the starting and ending points for the operation; Windows will automatically
	// adjust these values to align on the proper page size during VirtualProtect
	uintptr_t begin = uintptr_t(address);
	uintptr_t end = begin + length;

	// Scan the requested memory region and decommit/release the memory as is appropriate
	while(begin < end) {

		// Query the information about the current region to be released
		if(!VirtualQueryEx(m_host->ProcessHandle, reinterpret_cast<void*>(begin), &meminfo, sizeof(MEMORY_BASIC_INFORMATION)))
			throw LinuxException(LINUX_EACCES, Win32Exception());

		// Don't try to decommit pages that aren't already committed, munmap() should not
		// return an error if the page(s) aren't committed (MEM_FREE or MEM_RESERVE)
		if(meminfo.State == MEM_COMMIT) {

			//
			// TODO: Write-back for shared memory maps goes here, need to check if it's
			// dirty and the region flags indicate a write-back is necessary.  Determining
			// if it's shared can't be done with MEMORY_BASIC_INFORMATION since the current
			// intention is to do it all manually via VirtualAllocEx()
			//
			
			if(!VirtualFreeEx(m_host->ProcessHandle, reinterpret_cast<void*>(begin), min(end - begin, meminfo.RegionSize), MEM_DECOMMIT))
				throw LinuxException(LINUX_EINVAL, Win32Exception());
		}

		// If the current base address matches the allocation base address, this region can possibly be released
		if(meminfo.BaseAddress == meminfo.AllocationBase) {

			// Get updated information about the decommitted region
			MEMORY_BASIC_INFORMATION freeinfo;
			if(VirtualQueryEx(m_host->ProcessHandle, meminfo.AllocationBase, &freeinfo, sizeof(MEMORY_BASIC_INFORMATION))) {

				_ASSERTE(freeinfo.State != MEM_COMMIT);				// Should never happen

				// Don't attempt to release a region that is already MEM_FREE
				if(freeinfo.State == MEM_RESERVE) {

					// Find the original length of the allocation, and if the entire region is now decommitted, release it
					auto it = m_mappings.find(reinterpret_cast<void*>(begin));
					if((it != m_mappings.end()) && (it->second == freeinfo.RegionSize)) {

						if(!VirtualFreeEx(m_host->ProcessHandle, freeinfo.AllocationBase, 0, MEM_RELEASE)) throw LinuxException(LINUX_EINVAL, Win32Exception());
						m_mappings.unsafe_erase(it);
					}
				}
			}
		}

		begin += meminfo.RegionSize;		// Move to the next region
	}
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
	if(s_sysinfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) return;

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
// Process::Create (static)
//
// Constructs a new process instance from an ELF binary
//
// Arguments:
//
//	vm			- Reference to the VirtualMachine instance
//	handle		- Open FileSystem::Handle against the ELF binary to load
//	argv		- ELF command line arguments from caller
//	envp		- ELF environment variables from caller
//	hostpath	- Path to the external host to load
//	hostargs	- Command line arguments to pass to the host

template <ElfClass _class>
std::shared_ptr<Process> Process::Create(const std::shared_ptr<VirtualMachine>& vm, const FileSystem::HandlePtr& handle,
	const uapi::char_t** argv, const uapi::char_t** envp, const tchar_t* hostpath, const tchar_t* hostargs)
{
	using elf = elf_traits<_class>;

	std::unique_ptr<ElfImage>		executable;				// The main ELF binary image to be loaded
	std::unique_ptr<ElfImage>		interpreter;			// Optional interpreter image specified by executable
	uint8_t							random[16];				// 16-bytes of random data for AT_RANDOM auxvec
	StartupInfo						startinfo;				// Hosted process startup information

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
			FileSystem::HandlePtr interphandle = vm->OpenExecutable(executable->Interpreter);
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

		args.AppendAuxiliaryVector(LINUX_AT_PAGESZ, MemoryRegion::PageSize);					//  6
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
		(LINUX_AT_EXECFN);																		// 31 - TODO WHICH PATH? ARGUMENT TO EXECVE() OR THE ACTUAL PATH?
		(LINUX_AT_SYSINFO);																		// 32 - TODO MAY NOT IMPLEMENT?
		//args.AppendAuxiliaryVector(LINUX_AT_SYSINFO_EHDR, vdso->BaseAddress);					// 33 - TODO NEEDS VDSO

		// Generate the stack image for the process in it's address space
		ElfArguments::StackImage stackimg = args.GenerateStackImage<_class>(host->ProcessHandle);

		// Load the StartupInfo structure with the necessary information to get the ELF binary running
		startinfo.EntryPoint = (interpreter) ? interpreter->EntryPoint : executable->EntryPoint;
		startinfo.ProgramBreak = executable->ProgramBreak;
		startinfo.StackImage = stackimg.BaseAddress;
		startinfo.StackImageLength = stackimg.Length;

		// Create the Process object, transferring the host and startup information
		return std::make_shared<Process>(std::move(host), std::move(startinfo));
	}

	// Terminate the host process on exception since it doesn't get killed by the Host destructor
	// TODO: should be LinuxException wrapping the underlying one?  Only case right now where
	// that wouldn't be true is interpreter's OpenExec() call which should already throw LinuxExceptions
	catch(...) { host->Terminate(E_FAIL); throw; }
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
