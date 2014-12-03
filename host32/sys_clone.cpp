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
#include <linux\mman.h>

#pragma warning(push, 4)

// g_rpccontext (main.cpp)
//
// RPC context handle
extern sys32_context_t g_rpccontext;

//-----------------------------------------------------------------------------
// sys_clone
//
// TODO: WORDS

uapi::long_t sys_clone(unsigned long clone_flags, void* newsp, uapi::pid_t* parent_tidptr, int tls_val, uapi::pid_t* child_tidptr)
{
	// 0x012000011 = CLONE_CHILD_SETTID | CLONE_CHILD_CLEARTID | SIGCHLD
	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


