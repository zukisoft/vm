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

#ifndef __LINUX_AUXVEC_H_
#define __LINUX_AUXVEC_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/auxvec.h
//-----------------------------------------------------------------------------

#define LINUX_AT_NULL			0			/* end of vector */
#define LINUX_AT_IGNORE			1			/* entry should be ignored */
#define LINUX_AT_EXECFD			2			/* file descriptor of program */
#define LINUX_AT_PHDR			3			/* program headers for program */
#define LINUX_AT_PHENT			4			/* size of program header entry */
#define LINUX_AT_PHNUM			5			/* number of program headers */
#define LINUX_AT_PAGESZ			6			/* system page size */
#define LINUX_AT_BASE			7			/* base address of interpreter */
#define LINUX_AT_FLAGS			8			/* flags */
#define LINUX_AT_ENTRY			9			/* entry point of program */
#define LINUX_AT_NOTELF			10			/* program is not ELF */
#define LINUX_AT_UID			11			/* real uid */
#define LINUX_AT_EUID			12			/* effective uid */
#define LINUX_AT_GID			13			/* real gid */
#define LINUX_AT_EGID			14			/* effective gid */
#define LINUX_AT_PLATFORM		15			/* string identifying CPU for optimizations */
#define LINUX_AT_HWCAP			16			/* arch dependent hints at CPU capabilities */
#define LINUX_AT_CLKTCK			17			/* frequency at which times() increments */
											/* AT_* values 18 through 22 are reserved */
#define LINUX_AT_SECURE			23			/* secure mode boolean */
#define LINUX_AT_BASE_PLATFORM	24			/* string identifying real platform, may differ from AT_PLATFORM. */
#define LINUX_AT_RANDOM			25			/* address of 16 random bytes */
#define LINUX_AT_HWCAP2			26			/* extension of AT_HWCAP */
#define LINUX_AT_EXECFN			31			/* filename of program */

//-----------------------------------------------------------------------------
// arch/x86/include/uapi/asm/auxvec.h
//-----------------------------------------------------------------------------

#define LINUX_AT_SYSINFO			32
#define LINUX_AT_SYSINFO_EHDR		33

//-----------------------------------------------------------------------------
// extras
//-----------------------------------------------------------------------------

#if !defined(__midl) && defined(__cplusplus)
#include "elf.h"
namespace uapi {
	
	// Elf32_auxv_t
	//
	typedef struct _auxv_32 { 

	#ifdef __cplusplus
		// Convenience initializers for C++
		_auxv_32(Elf32_Addr type) : a_type(type), a_val(0) {}
		_auxv_32(Elf32_Addr type, Elf32_Addr val) : a_type(type), a_val(val) {}
	#endif

		Elf32_Addr		a_type;
		Elf32_Addr		a_val;

	} Elf32_auxv_t; 
 
	// Elf64_auxv_t
	//
	typedef struct _auxv_64 {

	#ifdef __cplusplus
		// Convenience initializers for C++
		_auxv_64(Elf64_Addr type) : a_type(type), a_val(0) {}
		_auxv_64(Elf64_Addr type, Elf64_Addr val) : a_type(type), a_val(val) {}
	#endif

		Elf64_Addr		a_type;
		Elf64_Addr		a_val;

	} Elf64_auxv_t; 

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif	// __LINUX_AUXVEC_H_
