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

#include "SystemCallContext.h"
#include "Thread.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_rt_sigreturn
//
// Returns from a signal handler and restores the thread
//
// Arguments:
//
//	context		- System call context object

uapi::long_t sys_rt_sigreturn(const Context* context)
{
	return -LINUX_ENOSYS;

	//context->Thread->EndSignal();
	//return 0;
}

// sys32_rt_sigreturn
//
sys32_long_t sys32_rt_sigreturn(sys32_context_t context)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_rt_sigreturn, context));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
