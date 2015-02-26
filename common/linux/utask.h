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

#ifndef __LINUX_UTASK_H_
#define __LINUX_UTASK_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// linux_utask32
//
// Clone of the 32-bit CONTEXT structure, used to collect and manipulate
// user-mode task state information
//-----------------------------------------------------------------------------

#define UTASK32_FLAGS_CONTROL		(0x000100000L | 0x00000001L)		// CONTEXT_CONTROL
#define UTASK32_FLAGS_INTEGER		(0x000100000L | 0x00000002L)		// CONTEXT_INTEGER
#define UTASK32_FLAGS_SEGMENTS		(0x000100000L | 0x00000004L)		// CONTEXT_SEGMENTS
#define UTASK32_FLAGS_FULL			(0x000100000L | 0x00000007L)		// CONTEXT_FULL

#pragma pack(push, 4)
typedef struct {

	uint32_t		flags;
	uint32_t		dr0;
	uint32_t		dr1;
	uint32_t		dr2;
	uint32_t		dr3;
	uint32_t		dr6;
	uint32_t		dr7;
	uint8_t			floatsave[112];
	uint32_t		gs;
	uint32_t		fs;
	uint32_t		es;
	uint32_t		ds;
	uint32_t		edi;
	uint32_t		esi;
	uint32_t		ebx;
	uint32_t		edx;
	uint32_t		ecx;
	uint32_t		eax;
	uint32_t		ebp;
	uint32_t		eip;
	uint32_t		cs;
	uint32_t		eflags;
	uint32_t		esp;
	uint32_t		ss;
	uint8_t			extendedregs[512];

} linux_utask32;
#pragma pack(pop)

typedef struct {

	uint64_t dummy;

} linux_utask64;


#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_utask32		utask32;
	typedef linux_utask64		utask64;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_UTASK_H_