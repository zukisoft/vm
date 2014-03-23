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

#pragma warning(push, 4)				// Enable maximum compiler warnings

// int mprotect(void *addr, size_t len, int prot);
//
// EBX	- void*		addr
// ECX	- size_t	len
// EDX	- int		prot
// ESI
// EDI
// EBP
//
int sys125_mprotect(PCONTEXT context)
{
	MEMORY_BASIC_INFORMATION		info;				// Virtual memory information

	// Pop out variables that will be used more than once in here
	void* address = reinterpret_cast<void*>(context->Ebx);
	size_t length = static_cast<size_t>(context->Ecx);

	// Request information about the allocated virtual memory region, which may be different
	if(VirtualQuery(address, &info, sizeof(MEMORY_BASIC_INFORMATION)) == 0) return -LINUX_EINVAL;

	// Check the length of the protection request against the allocated region
	if((uintptr_t(address) + length) > (uintptr_t(info.BaseAddress) + info.RegionSize)) return -LINUX_ENOMEM;

	// Attempt to set the equivalent set of protection to the requested memory region
	if(VirtualProtect(address, length, ProtToPageFlags(context->Edx), &context->Edx)) return 0;

	// ERROR_INVALID_ADDRESS -> -EINVAL; otherwise return -EACCES
	if(GetLastError() == ERROR_INVALID_ADDRESS) return -LINUX_EINVAL;
	else return -LINUX_EACCES;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
