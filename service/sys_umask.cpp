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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_umask
//
// Sets the default file creation mask for the calling process
//
// Arguments:
//
//	context		- SystemCall context object
//	mask		- New default file creation bitmask

__int3264 sys_umask(const SystemCall::Context* context, uapi::mode_t mask)
{
	_ASSERTE(context);

	try { 

		SystemCall::Impersonation impersonation;

		// Get the previously set UMASK and apply the new one
		uapi::mode_t previous = context->Process->FileCreationModeMask;
		context->Process->FileCreationModeMask = mask;

		// Return the previous UMASK bitmask as the result from this system call
		return static_cast<__int3264>(previous);
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_umask
//
sys32_long_t sys32_umask(sys32_context_t context, sys32_mode_t mask)
{
	return static_cast<sys32_long_t>(sys_umask(reinterpret_cast<SystemCall::Context*>(context), mask));
}

#ifdef _M_X64
// sys64_umask
//
sys64_long_t sys64_umask(sys64_context_t context, sys64_mode_t mask)
{
	return sys_umask(reinterpret_cast<SystemCall::Context*>(context), mask);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
