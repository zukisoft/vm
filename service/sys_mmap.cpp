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

//-----------------------------------------------------------------------------
// sys_mmap
//
// Maps files or devices into process memory
//
// Arguments:
//
//	context		- SystemCall context object
//	address		- Base address for the mapping, or null
//	length		- Length of the mapping
//	protection	- Memory protection flags to assign to the mapping
//	flags		- Flags and options
//	fd			- File/device from which to create the mapping
//	pgoffset	- Offset, in pages, into file/device from which to map

__int3264 sys_mmap(const SystemCall::Context* context, void* address, size_t length, int protection, int flags, int fd, uapi::off_t pgoffset)
{
	_ASSERTE(context);

	try { 		
		
		SystemCall::Impersonation impersonation;

		// MAP_PRIVATE and MAP_SHARED dictate how this system call will work
		switch(flags & (LINUX_MAP_PRIVATE | LINUX_MAP_SHARED)) {

			// MAP_PRIVATE - Create a private memory mapping directly in the hosted process
			//
			case LINUX_MAP_PRIVATE:			
				return reinterpret_cast<__int3264>(context->Process->MapMemory(address, length, protection, flags, fd, pgoffset * SystemInformation::PageSize));

			// MAP_SHARED - A virtual machine level shared memory region must be created or accessed
			//
			case LINUX_MAP_SHARED:
				throw LinuxException(LINUX_EINVAL);		// <--- TODO: LINUX_MAP_SHARED implementation

			default: throw LinuxException(LINUX_EINVAL);
		}
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_mmap
//
sys32_long_t sys32_mmap(sys32_context_t context, sys32_addr_t address, sys32_size_t length, sys32_int_t prot, sys32_int_t flags, sys32_int_t fd, sys32_off_t pgoffset)
{
	return static_cast<sys32_long_t>(sys_mmap(reinterpret_cast<SystemCall::Context*>(context), reinterpret_cast<void*>(address), length, prot, flags, fd, pgoffset));
}

#ifdef _M_X64
// sys64_mmap
//
sys64_long_t sys64_mmap(sys64_context_t context, sys64_addr_t address, sys64_size_t length, sys64_int_t prot, sys64_int_t flags, sys64_int_t fd, sys64_off_t pgoffset)
{
	return sys_mmap(reinterpret_cast<SystemCall::Context*>(context), reinterpret_cast<void*>(address), length, prot, flags, fd, pgoffset);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
