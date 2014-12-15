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
// sys_trace
//
// Receives a trace message from a hosted process
//
// Arguments:
//
//	context		- SystemCall context object
//	message		- ANSI message string
//	length		- Length of the ANSI message string

HRESULT sys_trace(const SystemCall::Context* context, const char_t* message, size_t length)
{
	_ASSERTE(context);

	UNREFERENCED_PARAMETER(context);
	UNREFERENCED_PARAMETER(length);

	// This can be turned into something more substantial later on, for now just spit it out
	OutputDebugStringA(message);

	return S_OK;
}

// sys32_trace
//
HRESULT sys32_trace(sys32_context_t context, sys32_char_t* message, sys32_size_t length)
{
	return sys_trace(reinterpret_cast<SystemCall::Context*>(context), message, length);
}

#ifdef _M_X64
// sys64_trace
//
HRESULT sys64_trace(sys64_context_t context, sys64_char_t* message, sys64_sizeis_t length)
{
	return sys_trace(reinterpret_cast<SystemCall::Context*>(context), message, length);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
