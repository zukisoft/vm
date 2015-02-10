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

#ifndef __LINUX_PTRACE_H_
#define __LINUX_PTRACE_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// arch/x86/include/uapi/asm/ptrace.h
//-----------------------------------------------------------------------------

// NOTE: This structure is not equivalent to the linux version and cannot be
// used as a parameter for system calls
typedef struct {

	// Integer Registers
	uint32_t		eax;
	uint32_t		ebx;
	uint32_t		ecx;
	uint32_t		edx;
	uint32_t		edi;
	uint32_t		esi;

	// Control Registers
	uint32_t		ebp;
	uint32_t		eip;
	uint32_t		esp;
	uint32_t		eflags;

} linux_pt_regs32;

// NOTE: This structure is not equivalent to the linux version and cannot be
// used as a parameter for system calls
typedef struct {

	// Integer Registers
	uint64_t		rax;
	uint64_t		rbx;
	uint64_t		rcx;
	uint64_t		rdx;
	uint64_t		rdi;
	uint64_t		rsi;
	uint64_t		r8;
	uint64_t		r9;
	uint64_t		r10;
	uint64_t		r11;
	uint64_t		r12;
	uint64_t		r13;
	uint64_t		r14;
	uint64_t		r15;

	// Control Registers
	uint64_t		rbp;
	uint64_t		rip;
	uint64_t		rsp;
	uint64_t		rflags;

} linux_pt_regs64;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_pt_regs32		pt_regs32;
	typedef linux_pt_regs64		pt_regs64;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_PTRACE_H_