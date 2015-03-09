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
#include "SystemCall.h"			// TODO: REMOVE ME? IS THIS A SYSTEM CALL OR NOT

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_rundown_context
//
// Releases a context object allocated by a client process/thread but was not 
// released prior to that process/thread terminating
//
// Arguments:
//
//	context		- System call context object to be rundown

void sys_rundown_context(Context* context)
{
	if(context == nullptr) return;

	// The hosted thread has died, remove it from the process
	context->Process->RemoveThread(context->Thread->ThreadId);

	// Release the context object and it's contained references
	Context::Release(context);
}

// sys32_context_t_rundown
//
void __RPC_USER sys32_context_t_rundown(sys32_context_t context)
{
	sys_rundown_context(reinterpret_cast<Context*>(context));
}

// sys32_context_exclusive_t_rundown
//
void __RPC_USER sys32_context_exclusive_t_rundown(sys32_context_exclusive_t context)
{
	sys_rundown_context(reinterpret_cast<Context*>(context));
}

#ifdef _M_X64
// sys64_context_t_rundown
//
void __RPC_USER sys32_context_t_rundown(sys64_context_t context)
{
	sys_rundown_context(reinterpret_cast<Context*>(context));
}

// sys64_context_exclusive_t_rundown
//
void __RPC_USER sys64_context_exclusive_t_rundown(sys64_context_exclusive_t context)
{
	sys_rundown_context(reinterpret_cast<Context*>(context));
}
#endif	// _M_X64

//---------------------------------------------------------------------------

#pragma warning(pop)
