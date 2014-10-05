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

#pragma warning(push, 4)

// g_rpccontext (main.cpp)
//
// RPC context handle
extern sys32_context_t g_rpccontext;

//
// SAMPLE FUNCTION
//

//-----------------------------------------------------------------------------
// int uname(struct utsname* buf)
//
// EBX	- struct utsname*	buf
// ECX
// EDX
// ESI
// EDI
// EBP
//
int sys000_template(PCONTEXT context)
{
	_ASSERTE(context->Eax == 122);			// Verify system call number

	return sys32_uname(g_rpccontext, reinterpret_cast<sys32_utsname*>(context->Ebx));
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
