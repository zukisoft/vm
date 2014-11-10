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

// sys_munmap
//
// Unmaps files or devices from process address space
__int3264 sys_munmap(const SystemCall::Context* context, void* address, uapi::size_t length)
{
	_ASSERTE(context);
	(length);
	(address);

	try { 		
		
		SystemCall::Impersonation impersonation; 
		///context->Process->TidAddress = address;
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	///return context->Process->ProcessId;
	return -1;
}

// sys32_munmap
//
sys32_long_t sys32_munmap(sys32_context_t context, sys32_addr_t address, sys32_size_t length)
{
	return static_cast<sys32_long_t>(sys_munmap(reinterpret_cast<SystemCall::Context*>(context), reinterpret_cast<void*>(address), length));
}

#ifdef _M_X64
// sys64_munmap
//
sys64_long_t sys64_munmap(sys64_context_t context, sys64_addr_t address, sys64_size_t length)
{
	return sys_munmap(reinterpret_cast<SystemCall::Context*>(context), reinterpret_cast<void*>(address), length);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
