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

#include "stdafx.h"						// Include project pre-compiled headers
#include "uapi.h"						// Include Linux UAPI declarations
#include "CriticalSection.h"			// Include CriticalSection declarations
#include "FileDescriptor.h"				// Include FileDescriptor declarations
#include "FileDescriptorTable.h"		// Include FileDescriptorTable decls
#include "SystemCall.h"					// Include SystemCall class declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

// Externs
extern int sys003_read(PCONTEXT);
extern int sys140__llseek(PCONTEXT);

// MMAP2_IMPL
//
// Function pointer to the appropriate mmap2() implementation
typedef int (*MMAP2_IMPL)(void*, size_t, uint32_t, uint32_t, int32_t, PLARGE_INTEGER);

//-----------------------------------------------------------------------------
// mmap2_private
//
// Implementation of mmap2() for private memory mapped files
//
// Arguments:
//
//	addr		- Optional base address for the mapping
//	length		- Length of the mapping in bytes
//	prot		- Memory protection flags (converted for Windows)
//	flags		- Additional memory mapping behavior flags
//	fd			- File to be mapped into virtual memory
//	offset		- Offset into the file to start the mapping

int mmap2_private(void* addr, size_t length, uint32_t prot, uint32_t flags, int32_t fd, PLARGE_INTEGER offset)
{
	DWORD			oldprotection;				// Previously set protection flags

	// Non-anonymous mappings require a valid file descriptor
	if(((flags & MAP_ANONYMOUS) == 0) && (fd <= 0)) return -LINUX_EBADF;
	
	// Get information about the requested memory region if address is not NULL
	MEMORY_BASIC_INFORMATION meminfo = { nullptr, nullptr, 0, length, MEM_FREE, 0, 0 };
	if(addr) VirtualQuery(addr, &meminfo, sizeof(MEMORY_BASIC_INFORMATION));

	// Verify that the requested length does not exceed the region length
	if(length > meminfo.RegionSize) return -LINUX_EINVAL;

	// Allocated region is already committed
	if(meminfo.State == MEM_COMMIT) {

		// The MAP_UNINITIALIZED optimization can be applied here by skipping the recommit
		// and just applying the updated protection flags to the memory region
		if((flags & MAP_UNINITIALIZED) == 0) {
		
			if(!VirtualFree(addr, length, MEM_DECOMMIT)) return -LINUX_EACCES;
			if(VirtualAlloc(addr, length, MEM_COMMIT, prot) != addr) return -LINUX_EACCES;
		}
		else if(!VirtualProtect(addr, length, prot, &oldprotection)) return -LINUX_EACCES;
	}

	// Allocated region is reserved but not committed (should technically never happen)
	else if(meminfo.State == MEM_RESERVE) {

		if(VirtualAlloc(addr, length, MEM_COMMIT, prot) != addr) return -LINUX_EACCES;
	}

	// Unallocated memory region, or input address was NULL
	else {

		// Generate the memory allocation flags from the input mmap flags
		uint32_t allocation = MEM_RESERVE | MEM_COMMIT;
		if(flags & MAP_HUGETLB) allocation |= MEM_LARGE_PAGES;
		if(flags & MAP_STACK) allocation |= MEM_TOP_DOWN;
		// TODO: MAP_GROWDOWN -- how does this work?

		// Allocate the memory region
		addr = VirtualAlloc(addr, length, allocation, prot);
		if(addr == nullptr) return -LINUX_EINVAL;
	}

	// Non-anonymous private mappings read memory contents from the source file
	if((flags & MAP_ANONYMOUS) == 0) {

		loff_t pointer;						// New file pointer position
		int result;							// Result from system call

		// The region needs to be set to PAGE_READWRITE for the data to be loaded
		VirtualProtect(addr, length, PAGE_READWRITE, &oldprotection);

		// Thunk into _llseek() to move the file pointer to the requested position
		result = SystemCall(sys140__llseek).Invoke(fd, offset->HighPart, offset->LowPart, &pointer, LINUX_SEEK_SET);
		if(result < 0) return result;
		if(pointer != offset->QuadPart) return -LINUX_EINVAL;

		// Thunk into read() to read the information from the file
		result = SystemCall(sys003_read).Invoke(fd, addr, length);
		if(result < 0) return result;
		if(static_cast<size_t>(result) != length) return -LINUX_EINVAL;

		// Restore the previous protection flags to the region after it's been loaded
		VirtualProtect(addr, length, oldprotection, &oldprotection);
	}

	// MAP_LOCKED must be applied after the region has been allocated
	if(flags & MAP_LOCKED) VirtualLock(addr, length);

	return reinterpret_cast<int>(addr);
}

//-----------------------------------------------------------------------------
// mmap2_shared
//
// Implementation of mmap2() for shared memory mapped files
//
// Arguments:
//
//	addr		- Optional base address for the mapping
//	length		- Length of the mapping in bytes
//	prot		- Memory protection flags (converted for Windows)
//	flags		- Additional memory mapping behavior flags
//	fd			- File to be mapped into virtual memory
//	offset		- Offset into the file to start the mapping

int mmap2_shared(void* addr, size_t length, uint32_t prot, uint32_t flags, int32_t fd, PLARGE_INTEGER offset)
{
	// Non-anonymous mappings require a valid file descriptor
	///if((flags & MAP_ANONYMOUS) && (fd == FileDescriptor::Null)) return -LINUX_EBADF;

	UNREFERENCED_PARAMETER(addr);
	UNREFERENCED_PARAMETER(length);
	UNREFERENCED_PARAMETER(prot);
	UNREFERENCED_PARAMETER(flags);
	UNREFERENCED_PARAMETER(fd);
	UNREFERENCED_PARAMETER(offset);

	return -1;
}

// void* mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset);
//
// EBX	- void*		addr
// ECX	- size_t	length
// EDX	- int		prot
// ESI	- int		flags
// EDI	- int		fd
// EBP	- off_t		pgoffset
//
int sys192_mmap2(PCONTEXT context)
{
	MMAP2_IMPL impl = nullptr;					// Implementation function pointer

	// Determine which version of mmap2 should be invoked based on MAP_PRIVATE | MAP_SHARED
	uint32_t flags = static_cast<uint32_t>(context->Esi);
	switch(flags & (MAP_PRIVATE | MAP_SHARED)) {

		case MAP_PRIVATE:	impl = mmap2_private; break;
		case MAP_SHARED:	impl = mmap2_shared; break;
		
		default: return -LINUX_EINVAL;
	}

	// Windows does not accept suggested addresses, remove it unless MAP_FIXED has been set
	void* address = (context->Esi & MAP_FIXED) ? reinterpret_cast<void*>(context->Ebx) : nullptr;

	// The length argument can never be zero
	size_t length = static_cast<size_t>(context->Ecx);
	if(length == 0) return -LINUX_EINVAL;

	// Offset is specified as 4096-byte pages
	LARGE_INTEGER offset;
	offset.QuadPart = static_cast<int64_t>(static_cast<uint64_t>(context->Ebp) << 12);

	// Invoke the proper version of this system call
	return impl(address, length, ProtToPageFlags(context->Edx), flags, static_cast<int32_t>(context->Edi), &offset);


	//// Cast out all of the arguments into local variables for clarity
	//void* addr = reinterpret_cast<void*>(context->Ebx);
	//size_t length = static_cast<size_t>(context->Ecx);
	//uint32_t prot = static_cast<uint32_t>(context->Edx);
	//uint32_t flags = static_cast<uint32_t>(context->Esi);
	//int32_t	fd = static_cast<int32_t>(context->Edi);

	//// Offset is specified as 4096-byte pages to allow for offsets greater than 2GiB on 32-bit systems
	//ULARGE_INTEGER offset;
	//offset.QuadPart = (context->Ebp * 4096);

	//// ANONYMOUS|SHARED and ANONYMOUS mappings use different implementations than a standard file mapping
	//if(flags & (MAP_ANONYMOUS | MAP_SHARED)) return map_anonymous_shared(addr, length, prot, flags, &offset);
	//else if(flags & MAP_ANONYMOUS) return map_anonymous(addr, length, prot, flags, &offset);

	//// blah
	//DWORD protection = ProtToPageFlags(context->Edx);
	//if(protection == PAGE_NOACCESS) protection = PAGE_READONLY;

	//// Attempt to create the file mapping using the arguments provided by the caller
	//HANDLE mapping = CreateFileMapping(oshandle, NULL, protection, 0, context->Ecx, NULL);
	//if(mapping) {

	//	// Convert the protection flags for MapViewOfFile(), once again avoiding PROT_NONE
	//	uint32_t filemapflags = ProtToFileMapFlags(context->Edx);
	//	if(filemapflags == 0) filemapflags = FILE_MAP_READ;
	//	if(flags & MAP_PRIVATE) filemapflags |= FILE_MAP_COPY;

	//	// Attempt to map the file into memory using the constructed flags and offset values
	//	void* address = MapViewOfFileEx(mapping, filemapflags, offset.HighPart, offset.LowPart, context->Ecx, 
	//		reinterpret_cast<void*>(context->Ebx));

	//	// Always close the file mapping handle, it will remain active until the view is unmapped
	//	CloseHandle(mapping);

	//	return (address) ? reinterpret_cast<int>(address) : -1;
	//}

	//// TODO: map error code
	//DWORD dw = GetLastError();

	//return -1;

	//// MAP_FIXED - interpret addr as absolute, otherwise it's a hint!
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
