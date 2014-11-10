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
#include "SystemCall.h"

#pragma warning(push, 4)

// sys_old_mmap
//
// Maps files or devices into memory
__int3264 sys_old_mmap(const SystemCall::Context* context, void* address, uapi::size_t length, int protection, int flags, int fd, uapi::off_t offset)
{
	_ASSERTE(context);
	(address);
	(length);
	(protection);
	(flags);
	(fd);
	(offset);

	try { 		
		
		SystemCall::Impersonation impersonation; 
		///context->Process->TidAddress = address;
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	///return context->Process->ProcessId;
	return -1;
}

// sys32_old_mmap
//
sys32_long_t sys32_old_mmap(sys32_context_t context, sys32_addr_t address, sys32_size_t length, sys32_int_t prot, sys32_int_t flags, sys32_int_t fd, sys32_off_t offset)
{
	//return static_cast<sys32_long_t>(sys_set_tid_address(reinterpret_cast<SystemCall::Context*>(context), reinterpret_cast<void*>(address)));
	return -1;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
