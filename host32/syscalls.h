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

#ifndef __SYSCALLS_H_
#define __SYSCALLS_H_
#pragma once

#include <linux/errno.h>
#include <linux/ldt.h>
#include <linux/signal.h>

#pragma warning(push, 4)

// syscall_t
//
// Prototype for a system call handle
using syscall_t = int (*)(PCONTEXT);

// g_syscalls
//
// Table of system calls, organized by entry point ordinal
extern syscall_t g_syscalls[512];

// TODO: PUT FUNCTION PROTOTYPES FOR EACH ONE HERE
extern uapi::long_t sys_noentry(PCONTEXT);

/* 001 */ extern uapi::long_t sys_exit(int status);
/* 002 */ extern uapi::long_t sys_fork(PCONTEXT);
/* 120 */ extern uapi::long_t sys_clone(PCONTEXT);
/* 174 */ extern uapi::long_t sys_rt_sigaction(int signal, const uapi::sigaction* action, uapi::sigaction* oldaction, size_t sigsetsize);
/* 190 */ extern uapi::long_t sys_vfork(PCONTEXT);
/* 243 */ extern uapi::long_t sys_set_thread_area(uapi::user_desc*);
/* 252 */ extern uapi::long_t sys_exit_group(int status);

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSCALLS_H_
