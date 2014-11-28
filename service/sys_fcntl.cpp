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

//-----------------------------------------------------------------------------
// sys_fcntl
//
// WORDS

__int3264 sys_fcntl(const SystemCall::Context* context, int fd, int cmd, void* arg)
{
	_ASSERTE(context);
	(arg);

	try { 		
		
		SystemCall::Impersonation impersonation;

		auto handle = context->Process->GetHandle(fd);

		switch(cmd) {

			case 0:	
				// testing - need access to original flags
				return context->Process->AddHandle(handle->Duplicate(0));
		}
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return -LINUX_EINVAL;
}

// sys32_fcntl64
//
sys32_long_t sys32_fcntl64(sys32_context_t context, sys32_int_t fd, sys32_int_t cmd, sys32_addr_t arg)
{
	return static_cast<sys32_long_t>(sys_fcntl(reinterpret_cast<SystemCall::Context*>(context), fd, cmd, reinterpret_cast<void*>(arg)));
}

#ifdef _M_X64
// sys64_fcntl
//
sys64_long_t sys64_fcntl(sys64_context_t context, sys64_int_t fd, sys64_int_t cmd, sys64_addr_t arg)
{
	return sys_fcntl(reinterpret_cast<SystemCall::Context*>(context), fd, cmd, reinterpret_cast<void*>(arg));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
