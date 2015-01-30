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
#include "SystemCall.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

// sys_mmap.cpp
//
uapi::long_t sys_mmap(const Context* context, void* address, size_t length, int protection, int flags, int fd, uapi::off_t pgoffset);

//-----------------------------------------------------------------------------
// sys_old_mmap
//
// Maps files or devices into process memory
//
// Arguments:
//
//	context		- System call context object
//	address		- Base address for the mapping, or null
//	length		- Length of the mapping
//	protection	- Memory protection flags to assign to the mapping
//	flags		- Flags and options
//	fd			- File/device from which to create the mapping
//	offset		- Offset into file/device from which to map

uapi::long_t sys_old_mmap(const Context* context, void* address, size_t length, int protection, int flags, int fd, uapi::off_t offset)
{
	// Compatibility function; the offset must be a multiple of the system page size
	if(offset & (SystemInformation::PageSize - 1)) return -LINUX_EINVAL;

	// sys_old_mmap() is equivalent to sys_mmap() with the offset in pages rather than bytes
	return sys_mmap(context, address, length, protection, flags, fd, offset / SystemInformation::PageSize);
}

// sys32_old_mmap
//
sys32_long_t sys32_old_mmap(sys32_context_t context, sys32_addr_t address, sys32_size_t length, sys32_int_t prot, sys32_int_t flags, sys32_int_t fd, sys32_off_t offset)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_old_mmap, context, reinterpret_cast<void*>(address), length, prot, flags, fd, offset));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
