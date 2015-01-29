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
// sys32_context_exclusive_t_rundown
//
// Releases a context handle allocated by a client process but was not released
// prior to the process terminating
//
// Arguments:
//
//	context			- [in] RPC context handle to be rundown

void __RPC_USER sys32_context_exclusive_t_rundown(sys32_context_exclusive_t context)
{
	if(context == nullptr) return;

	// TODO: CLEAN ME UP - TESTING CLOSEPROCESS - same pointer needed below
	ContextHandle* syscallcontext = reinterpret_cast<ContextHandle*>(context);
	syscallcontext->VirtualMachine->CloseProcess(syscallcontext->Process);

	// Cast the context handle back into a Context handle and destroy it
	ContextHandle::Release(reinterpret_cast<ContextHandle*>(context));
}

#ifdef _M_X64
//-----------------------------------------------------------------------------
// sys64_context_exclusive_t_rundown
//
// Releases a context handle allocated by a client process but was not released
// prior to the process terminating
//
// Arguments:
//
//	context			- [in] RPC context handle to be rundown

void __RPC_USER sys64_context_exclusive_t_rundown(sys64_context_exclusive_t context)
{
	if(context == nullptr) return;

	// Cast the context handle back into a Context handle and destroy it
	ContextHandle::Release(reinterpret_cast<ContextHandle*>(context));
}
#endif	// _M_X64

//---------------------------------------------------------------------------

#pragma warning(pop)
