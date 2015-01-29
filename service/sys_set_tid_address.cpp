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
#include "ContextHandle.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_set_tid_address
//
// Sets a pointer to the thread id.  See set_tid_address(2) for more details.
//
// Arguments:
//
//	context		- SystemCall context object
//	tidptr		- Address to assign as clear_child_tid

__int3264 sys_set_tid_address(const ContextHandle* context, void* tidptr)
{
	_ASSERTE(context);

	try { 		
		
		context->Process->TidAddress = tidptr;
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return context->Process->ProcessId;
}

// sys32_set_tid_address
//
sys32_long_t sys32_set_tid_address(sys32_context_t context, sys32_addr_t tidptr)
{
	return static_cast<sys32_long_t>(sys_set_tid_address(reinterpret_cast<ContextHandle*>(context), reinterpret_cast<void*>(tidptr)));
}

#ifdef _M_X64
// sys64_set_tid_address
//
sys64_long_t sys64_set_tid_address(sys64_context_t context, sys64_addr_t tidptr)
{
	return sys_set_tid_address(reinterpret_cast<ContextHandle*>(context), reinterpret_cast<void*>(tidptr));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
