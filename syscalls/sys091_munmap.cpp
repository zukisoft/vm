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

#pragma warning(push, 4)				// Enable maximum compiler warnings

// int munmap(void *addr, size_t length)
//
// EBX	- void*			addr
// ECX	- size_t		length
// EDX
// ESI
// EDI
// EBP
//
int sys091_munmap(PCONTEXT context)
{
	BOOL					result;				// Result from function call

	_ASSERTE(context->Eax == 91);				// Verify system call number

	// Cast out the address and length parameters
	void* addr = reinterpret_cast<void*>(context->Ebx);
	size_t length = static_cast<size_t>(context->Ecx);

	// Get information about the mapped memory region
	MEMORY_BASIC_INFORMATION meminfo;
	if(!VirtualQuery(addr, &meminfo, sizeof(MEMORY_BASIC_INFORMATION))) return -LINUX_EINVAL;

	// Check that the memory has actually been allocated
	if(meminfo.State == MEM_FREE) return -LINUX_EINVAL;

	// DEBUG: Verify addr matches the base address of the allocated region
	if(addr != meminfo.AllocationBase)
		_RPTF2(_CRT_ASSERT, "munmap: address %p does not match allocated region base address %p", addr, meminfo.AllocationBase);

	// DEBUG: Verify length matches the size of the allocated region
	if(length != meminfo.RegionSize) 
		_RPTF2(_CRT_ASSERT, "munmap: length %u does not match allocated region size %u", length, meminfo.RegionSize);
		
	// The memory allocated by mmap/mmap2 may have been done with MapViewOfFile
	// or VirtualAlloc; the correct function must be called to release it
	if(meminfo.Type == MEM_MAPPED) result = UnmapViewOfFile(addr);
	else result = VirtualFree(addr, 0, MEM_RELEASE);

	return (result) ? 0 : -LINUX_EINVAL;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
