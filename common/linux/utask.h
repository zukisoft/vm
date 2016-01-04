//-----------------------------------------------------------------------------
// Copyright (c) 2016 Michael G. Brehm
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

#pragma warning(disable:4201)		// nameless struct / union

// linux_m128a
//
// Structure for a signed 128-bit register value
typedef struct {
    
	uint64_t		low;
	int64_t			high;

} linux_m128a;

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

	uint32_t				flags;

	uint32_t				dr0;
	uint32_t				dr1;
	uint32_t				dr2;
	uint32_t				dr3;
	uint32_t				dr6;
	uint32_t				dr7;

	struct {

		uint32_t			controlword;
		uint32_t			statusword;
		uint32_t			tagword;
		uint32_t			erroroffset;
		uint32_t			errorselector;
		uint32_t			dataoffset;
		uint32_t			dataselector;
		uint8_t				registerarea[80];
		uint32_t			spare0;
	
	} fltsave;

	uint32_t				gs;
	uint32_t				fs;
	uint32_t				es;
	uint32_t				ds;

	uint32_t				edi;
	uint32_t				esi;
	uint32_t				ebx;
	uint32_t				edx;
	uint32_t				ecx;
	uint32_t				eax;

	uint32_t				ebp;
	uint32_t				eip;
	uint32_t				cs;
	uint32_t				eflags;
	uint32_t				esp;
	uint32_t				ss;

	uint8_t					extendedregisters[512];

} linux_utask32;
#pragma pack(pop)

//-----------------------------------------------------------------------------
// linux_utask64
//
// Clone of the 64-bit CONTEXT structure, used to collect and manipulate
// user-mode task state information
//-----------------------------------------------------------------------------

#define UTASK64_FLAGS_CONTROL			(0x00100000L | 0x00000001L)		// CONTEXT_CONTROL
#define UTASK64_FLAGS_INTEGER			(0x00100000L | 0x00000002L)		// CONTEXT_INTEGER
#define UTASK64_FLAGS_SEGMENTS			(0x00100000L | 0x00000004L)		// CONTEXT_SEGMENTS
#define UTASK64_FLAGS_FLOATING_POINT	(0x00100000L | 0x00000008L)		// CONTEXT_FLOATING_POINT
#define UTASK64_FLAGS_FULL				(0x00100000L | 0x0000000BL)		// CONTEXT_FULL

typedef struct {

	uint64_t				p1home;
	uint64_t				p2home;
	uint64_t				p3home;
	uint64_t				p4home;
	uint64_t				p5home;
	uint64_t				p6home;

	uint32_t				flags;
	uint32_t				mxcsr;

	uint16_t				cs;
	uint16_t				ds;
	uint16_t				es;
	uint16_t				fs;
	uint16_t				gs;
	uint16_t				ss;
	uint32_t				eflags;

	uint64_t				dr0;
	uint64_t				dr1;
	uint64_t				dr2;
	uint64_t				dr3;
	uint64_t				dr6;
	uint64_t				dr7;

	uint64_t				rax;
	uint64_t				rcx;
	uint64_t				rdx;
	uint64_t				rbx;
	uint64_t				rsp;
	uint64_t				rbp;
	uint64_t				rsi;
	uint64_t				rdi;
	uint64_t				r8;
	uint64_t				r9;
	uint64_t				r10;
	uint64_t				r11;
	uint64_t				r12;
	uint64_t				r13;
	uint64_t				r14;
	uint64_t				r15;

	uint64_t				rip;

	union {

		struct {

			uint16_t		controlword;
			uint16_t		statusword;
			uint8_t			tagword;
			uint8_t			reserved1;
			uint16_t		erroropcode;
			uint32_t		erroroffset;
			uint16_t		errorselector;
			uint16_t		reserved2;
			uint32_t		dataoffset;
			uint16_t		dataselector;
			uint16_t		reserved3;
			uint32_t		mxcsr;
			uint32_t		mxcsrmask;
			linux_m128a		floatregisters[8];
			linux_m128a		xmmregisters[16];
			uint8_t			reserved4[96];

		} fltsave;

		struct {

			linux_m128a		header[2];
			linux_m128a		legacy[8];
			linux_m128a		xmm0;
			linux_m128a		xmm1;
			linux_m128a		xmm2;
			linux_m128a		xmm3;
			linux_m128a		xmm4;
			linux_m128a		xmm5;
			linux_m128a		xmm6;
			linux_m128a		xmm7;
			linux_m128a		xmm8;
			linux_m128a		xmm9;
			linux_m128a		xmm10;
			linux_m128a		xmm11;
			linux_m128a		xmm12;
			linux_m128a		xmm13;
			linux_m128a		xmm14;
			linux_m128a		xmm15;
		};
	};

	linux_m128a				vectorregister[26];
	uint64_t				vectorcontrol;

	uint64_t				debugcontrol;
	uint64_t				lastbranchtorip;
	uint64_t				lastbranchfromrip;
	uint64_t				lastexceptiontorip;
	uint64_t				lastexceptionfromrip;

} linux_utask64;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_utask32						utask32;
	typedef __declspec(align(16)) linux_utask64	utask64;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#pragma warning(default:4201)	// nameless struct / union

#endif		// __LINUX_UTASK_H_