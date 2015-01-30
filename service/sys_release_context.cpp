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
#include "SystemCall.h"			// TODO: REMOVE ME?

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys32_release_context
//
// Releases a context handle previously allocated with sys32_allocate_context
//
// Arguments:
//
//	context			- [in/out] contains the handle to release and will be set to null

HRESULT sys32_release_context(sys32_context_exclusive_t* context)
{
	if((context == nullptr) || (*context == nullptr)) return E_POINTER;

	// TODO: CLEAN ME UP - TESTING CLOSEPROCESS - same pointer needed below
	Context* syscallcontext = reinterpret_cast<Context*>(*context);
	syscallcontext->VirtualMachine->CloseProcess(syscallcontext->Process);

	// Cast the context handle back into a Context handle and destroy it
	Context::Release(syscallcontext);

	*context = nullptr;					// Reset context handle to null
	return S_OK;						// Context has been released
}

#ifdef _M_X64
//-----------------------------------------------------------------------------
// sys64_release_context
//
// Releases a context handle previously allocated with sys64_allocate_context
//
// Arguments:
//
//	context			- [in/out] contains the handle to release and will be set to null

HRESULT sys64_release_context(sys64_context_exclusive_t* context)
{
	if((context == nullptr) || (*context == nullptr)) return E_POINTER;

	// Cast the context handle back into a Context handle and destroy it
	Context::Release(reinterpret_cast<Context*>(*context));

	*context = nullptr;					// Reset context handle to null
	return S_OK;						// Context has been released
}
#endif	// _M_X64

//---------------------------------------------------------------------------

#pragma warning(pop)
