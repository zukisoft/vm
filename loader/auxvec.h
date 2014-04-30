#ifndef __AUXVEC_H_
#define __AUXVEC_H_
#pragma once

#include "elf.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------

/* Symbolic values for the entries in the auxiliary table
   put on the initial stack */

#define AT_NULL				0			/* end of vector */
#define AT_IGNORE			1			/* entry should be ignored */
#define AT_EXECFD			2			/* file descriptor of program */
#define AT_PHDR				3			/* program headers for program */
#define AT_PHENT			4			/* size of program header entry */
#define AT_PHNUM			5			/* number of program headers */
#define AT_PAGESZ			6			/* system page size */
#define AT_BASE				7			/* base address of interpreter */
#define AT_FLAGS			8			/* flags */
#define AT_ENTRY			9			/* entry point of program */
#define AT_NOTELF			10			/* program is not ELF */
#define AT_UID				11			/* real uid */
#define AT_EUID				12			/* effective uid */
#define AT_GID				13			/* real gid */
#define AT_EGID				14			/* effective gid */
#define AT_PLATFORM			15			/* string identifying CPU for optimizations */
#define AT_HWCAP			16			/* arch dependent hints at CPU capabilities */
#define AT_CLKTCK			17			/* frequency at which times() increments */
										/* AT_* values 18 through 22 are reserved */
#define AT_SECURE			23			/* secure mode boolean */
#define AT_BASE_PLATFORM	24			/* string identifying real platform, may
										/* differ from AT_PLATFORM. */
#define AT_RANDOM			25			/* address of 16 random bytes */
#define AT_HWCAP2			26			/* extension of AT_HWCAP */

#define AT_EXECFN			31			/* filename of program */
#define AT_SYSINFO			32
#define AT_SYSINFO_EHDR		33

//
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
 
//
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

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __AUXVEC_H_
