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
// sys_brk
//
// Sets the program break address for a process
//
// Arguments:
//
//	context			- SystemCall context object
//	brk				- Requested new program break address

__int3264 sys_brk(const ContextHandle* context, void* brk)
{
	_ASSERTE(context);

	try { 

		// Request the new program break address from the Process object
		return reinterpret_cast<__int3264>(context->Process->SetProgramBreak(brk));
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_brk
//
sys32_long_t sys32_brk(sys32_context_t context, sys32_addr_t brk)
{
	return static_cast<sys32_long_t>(sys_brk(reinterpret_cast<ContextHandle*>(context), reinterpret_cast<void*>(brk)));
}

#ifdef _M_X64
// sys64_brk
//
sys64_long_t sys64_brk(sys64_context_t context, sys64_addr_t brk)
{
	return sys_brk(reinterpret_cast<ContextHandle*>(context), reinterpret_cast<void*>(brk));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
