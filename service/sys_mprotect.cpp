//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#include "SystemCallContext.h"
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_mprotect
//
// Assigns the protection flags for a region of memory
//
// Arguments:
//
//	context		- System call context object
//	address		- Base address from which to apply the protection
//	length		- Length of the region to apply the protection
//	prot		- Memory protection flags

uapi::long_t sys_mprotect(const Context* context, void* address, uapi::size_t length, int prot)
{
	return -LINUX_ENOSYS;

	//context->Process->ProtectMemory(address, length, prot);
	//return 0;
}

// sys32_mprotect
//
sys32_long_t sys32_mprotect(sys32_context_t context, sys32_addr_t address, sys32_size_t length, sys32_int_t prot)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_mprotect, context, reinterpret_cast<void*>(address), length, prot));
}

#ifdef _M_X64
// sys64_mprotect
//
sys64_long_t sys64_mprotect(sys64_context_t context, sys64_addr_t address, sys64_size_t length, sys64_int_t prot)
{
	return SystemCall::Invoke(sys_mprotect, context, reinterpret_cast<void*>(address), length, prot);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
