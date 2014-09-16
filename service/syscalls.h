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

#ifndef __SYSCALLS_H_
#define __SYSCALLS_H_
#pragma once

#include "RpcInterface.h"

#pragma warning(push, 4)

// syscall32_listener
// syscall64_listener
//
// Type declaration for RpcInterface<> template that maps the typeid and entry point vector
#ifndef _M_X64

// 32-bit listener
#include <syscalls32.h>
extern SystemCalls32_v1_0_epv_t syscalls32_epv32;
using syscall32_listener = RpcInterface<&SystemCalls32_v1_0_s_ifspec, &EPVID_SYSTEMCALLS32, &syscalls32_epv32>;

#else

// 32-bit listener
#include <syscalls32.h>
extern SystemCalls32_v1_0_epv_t syscalls32_epv64;
using syscall32_listener = RpcInterface<&SystemCalls32_v1_0_s_ifspec, &EPVID_SYSTEMCALLS32, &syscalls32_epv64>;

// 64-bit listener
#include <syscalls64.h>
extern SystemCalls64_v1_0_epv_t syscalls64_epv64;
using syscall64_listener = RpcInterface<&SystemCalls64_v1_0_s_ifspec, &EPVID_SYSTEMCALLS64, &syscalls64_epv64>;

#endif

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSCALLS_H_
