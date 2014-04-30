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

#ifndef __LINUX_ELF_EM_H_
#define __LINUX_ELF_EM_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/elf-em.h
//-----------------------------------------------------------------------------

#define LINUX_EM_NONE			0
#define LINUX_EM_M32			1
#define LINUX_EM_SPARC			2
#define LINUX_EM_386			3
#define LINUX_EM_68K			4
#define LINUX_EM_88K			5
#define LINUX_EM_486			6		/* Perhaps disused */
#define LINUX_EM_860			7
#define LINUX_EM_MIPS			8		/* MIPS R3000 (officially, big-endian only) */
#define LINUX_EM_MIPS_RS3_LE	10		/* MIPS R3000 little-endian */
#define LINUX_EM_MIPS_RS4_BE	10		/* MIPS R4000 big-endian */
#define LINUX_EM_PARISC			15		/* HPPA */
#define LINUX_EM_SPARC32PLUS	18		/* Sun's "v8plus" */
#define LINUX_EM_PPC			20		/* PowerPC */
#define LINUX_EM_PPC64			21		/* PowerPC64 */
#define LINUX_EM_SPU			23		/* Cell BE SPU */
#define LINUX_EM_ARM			40		/* ARM 32 bit */
#define LINUX_EM_SH				42		/* SuperH */
#define LINUX_EM_SPARCV9		43		/* SPARC v9 64-bit */
#define LINUX_EM_IA_64			50		/* HP/Intel IA-64 */
#define LINUX_EM_X86_64			62		/* AMD x86-64 */
#define LINUX_EM_S390			22		/* IBM S/390 */
#define LINUX_EM_CRIS			76		/* Axis Communications 32-bit embedded processor */
#define LINUX_EM_V850			87		/* NEC v850 */
#define LINUX_EM_M32R			88		/* Renesas M32R */
#define LINUX_EM_MN10300		89		/* Panasonic/MEI MN10300, AM33 */
#define LINUX_EM_BLACKFIN		106     /* ADI Blackfin Processor */
#define LINUX_EM_TI_C6000		140		/* TI C6X DSPs */
#define LINUX_EM_AARCH64		183		/* ARM 64 bit */
#define LINUX_EM_FRV			0x5441	/* Fujitsu FR-V */
#define LINUX_EM_AVR32			0x18AD	/* Atmel AVR32 */
#define LINUX_EM_ALPHA			0x9026
#define LINUX_EM_CYGNUS_V850	0x9080
#define LINUX_EM_CYGNUS_M32R	0x9041
#define LINUX_EM_S390_OLD		0xA390
#define LINUX_EM_CYGNUS_MN10300 0xBEEF

//-----------------------------------------------------------------------------

#endif	// __LIUX_ELF_EM_H_
