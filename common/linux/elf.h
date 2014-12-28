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

#ifndef __LINUX_ELF_H_
#define __LINUX_ELF_H_
#pragma once

#include "types.h"

#pragma warning(push, 4)				

//-----------------------------------------------------------------------------
// include/uapi/linux/elf.h
//-----------------------------------------------------------------------------

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	/* 32-bit ELF base types. */
	typedef uint32_t				Elf32_Addr;
	typedef uint16_t				Elf32_Half;
	typedef uint32_t				Elf32_Off;
	typedef int32_t					Elf32_Sword;
	typedef uint32_t				Elf32_Word;

	/* 64-bit ELF base types. */
	typedef uint64_t				Elf64_Addr;
	typedef uint16_t				Elf64_Half;
	typedef int16_t					Elf64_SHalf;
	typedef uint64_t				Elf64_Off;
	typedef int32_t					Elf64_Sword;
	typedef uint32_t				Elf64_Word;
	typedef uint64_t				Elf64_Xword;
	typedef int64_t					Elf64_Sxword;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

/* These constants are for the segment types stored in the image headers */
#define LINUX_PT_NULL			0
#define LINUX_PT_LOAD			1
#define LINUX_PT_DYNAMIC		2
#define LINUX_PT_INTERP			3
#define LINUX_PT_NOTE			4
#define LINUX_PT_SHLIB			5
#define LINUX_PT_PHDR			6
#define LINUX_PT_TLS			7               /* Thread local storage segment */
#define LINUX_PT_LOOS			0x60000000      /* OS-specific */
#define LINUX_PT_HIOS			0x6fffffff      /* OS-specific */
#define LINUX_PT_LOPROC			0x70000000
#define LINUX_PT_HIPROC			0x7fffffff
#define LINUX_PT_GNU_EH_FRAME	0x6474e550
#define LINUX_PT_GNU_STACK		(LINUX_PT_LOOS + 0x474e551)

/*
 * Extended Numbering
 *
 * If the real number of program header table entries is larger than
 * or equal to PN_XNUM(0xffff), it is set to sh_info field of the
 * section header at index 0, and PN_XNUM is set to e_phnum
 * field. Otherwise, the section header at index 0 is zero
 * initialized, if it exists.
 *
 * Specifications are available in:
 *
 * - Oracle: Linker and Libraries.
 *   Part No: 817–1984–19, August 2011.
 *   http://docs.oracle.com/cd/E18752_01/pdf/817-1984.pdf
 *
 * - System V ABI AMD64 Architecture Processor Supplement
 *   Draft Version 0.99.4,
 *   January 13, 2010.
 *   http://www.cs.washington.edu/education/courses/cse351/12wi/supp-docs/abi.pdf
 */
#define LINUX_PN_XNUM			0xffff

/* These constants define the different elf file types */
#define LINUX_ET_NONE			0
#define LINUX_ET_REL			1
#define LINUX_ET_EXEC			2
#define LINUX_ET_DYN			3
#define LINUX_ET_CORE			4
#define LINUX_ET_LOPROC			0xff00
#define LINUX_ET_HIPROC			0xffff

/* This is the info that is needed to parse the dynamic section of the file */
#define LINUX_DT_NULL			0
#define LINUX_DT_NEEDED			1
#define LINUX_DT_PLTRELSZ		2
#define LINUX_DT_PLTGOT			3
#define LINUX_DT_HASH			4
#define LINUX_DT_STRTAB			5
#define LINUX_DT_SYMTAB			6
#define LINUX_DT_RELA			7
#define LINUX_DT_RELASZ			8
#define LINUX_DT_RELAENT		9
#define LINUX_DT_STRSZ			10
#define LINUX_DT_SYMENT			11
#define LINUX_DT_INIT			12
#define LINUX_DT_FINI			13
#define LINUX_DT_SONAME			14
#define LINUX_DT_RPATH 			15
#define LINUX_DT_SYMBOLIC		16
#define LINUX_DT_REL	        17
#define LINUX_DT_RELSZ			18
#define LINUX_DT_RELENT			19
#define LINUX_DT_PLTREL			20
#define LINUX_DT_DEBUG			21
#define LINUX_DT_TEXTREL		22
#define LINUX_DT_JMPREL			23
#define LINUX_DT_ENCODING		32
#define LINUX_OLD_DT_LOOS		0x60000000
#define LINUX_DT_LOOS			0x6000000d
#define LINUX_DT_HIOS			0x6ffff000
#define LINUX_DT_VALRNGLO		0x6ffffd00
#define LINUX_DT_VALRNGHI		0x6ffffdff
#define LINUX_DT_ADDRRNGLO		0x6ffffe00
#define LINUX_DT_ADDRRNGHI		0x6ffffeff
#define LINUX_DT_VERSYM			0x6ffffff0
#define LINUX_DT_RELACOUNT		0x6ffffff9
#define LINUX_DT_RELCOUNT		0x6ffffffa
#define LINUX_DT_FLAGS_1		0x6ffffffb
#define LINUX_DT_VERDEF			0x6ffffffc
#define	LINUX_DT_VERDEFNUM		0x6ffffffd
#define LINUX_DT_VERNEED		0x6ffffffe
#define	LINUX_DT_VERNEEDNUM		0x6fffffff
#define LINUX_OLD_DT_HIOS		0x6fffffff
#define LINUX_DT_LOPROC			0x70000000
#define LINUX_DT_HIPROC			0x7fffffff

/* This info is needed when parsing the symbol table */
#define LINUX_STB_LOCAL			0
#define LINUX_STB_GLOBAL		1
#define LINUX_STB_WEAK			2

#define LINUX_STT_NOTYPE		0
#define LINUX_STT_OBJECT		1
#define LINUX_STT_FUNC			2
#define LINUX_STT_SECTION		3
#define LINUX_STT_FILE			4
#define LINUX_STT_COMMON		5
#define LINUX_STT_TLS			6

#define LINUX_ELF_ST_BIND(x)	((x) >> 4)
#define LINUX_ELF_ST_TYPE(x)	(((unsigned int) x) & 0xf)
#define LINUX_ELF32_ST_BIND(x)	LINUX_ELF_ST_BIND(x)
#define LINUX_ELF32_ST_TYPE(x)	LINUX_ELF_ST_TYPE(x)
#define LINUX_ELF64_ST_BIND(x)	LINUX_ELF_ST_BIND(x)
#define LINUX_ELF64_ST_TYPE(x)	LINUX_ELF_ST_TYPE(x)

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef struct dynamic {
	  Elf32_Sword	d_tag;
	  union {
		Elf32_Sword	d_val;
		Elf32_Addr	d_ptr;
	  } d_un;
	} Elf32_Dyn;

	typedef struct {
	  Elf64_Sxword	d_tag;		/* entry tag value */
	  union {
		Elf64_Xword	d_val;
		Elf64_Addr	d_ptr;
	  } d_un;
	} Elf64_Dyn;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

/* The following are used with relocations */
#define LINUX_ELF32_R_SYM(x)	((x) >> 8)
#define LINUX_ELF32_R_TYPE(x)	((x) & 0xff)
#define LINUX_ELF64_R_SYM(i)	((i) >> 32)
#define LINUX_ELF64_R_TYPE(i)	((i) & 0xffffffff)

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef struct elf32_rel {
	  Elf32_Addr	r_offset;
	  Elf32_Word	r_info;
	} Elf32_Rel;

	typedef struct elf64_rel {
	  Elf64_Addr	r_offset;		/* Location at which to apply the action */
	  Elf64_Xword	r_info;			/* index and type of relocation */
	} Elf64_Rel;

	typedef struct elf32_rela{
	  Elf32_Addr	r_offset;
	  Elf32_Word	r_info;
	  Elf32_Sword	r_addend;
	} Elf32_Rela;

	typedef struct elf64_rela {
	  Elf64_Addr	r_offset;		/* Location at which to apply the action */
	  Elf64_Xword	r_info;			/* index and type of relocation */
	  Elf64_Sxword	r_addend;		/* Constant addend used to compute value */
	} Elf64_Rela;

	typedef struct elf32_sym{
	  Elf32_Word	st_name;
	  Elf32_Addr	st_value;
	  Elf32_Word	st_size;
	  unsigned char	st_info;
	  unsigned char	st_other;
	  Elf32_Half	st_shndx;
	} Elf32_Sym;

	typedef struct elf64_sym {
	  Elf64_Word	st_name;		/* Symbol name, index in string tbl */
	  unsigned char	st_info;		/* Type and binding attributes */
	  unsigned char	st_other;		/* No defined meaning, 0 */
	  Elf64_Half	st_shndx;		/* Associated section index */
	  Elf64_Addr	st_value;		/* Value of the symbol */
	  Elf64_Xword	st_size;		/* Associated symbol size */
	} Elf64_Sym;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

#define LINUX_EI_NIDENT			16

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef struct elf32_hdr {
	  unsigned char	e_ident[LINUX_EI_NIDENT];
	  Elf32_Half	e_type;
	  Elf32_Half	e_machine;
	  Elf32_Word	e_version;
	  Elf32_Addr	e_entry;  /* Entry point */
	  Elf32_Off		e_phoff;
	  Elf32_Off		e_shoff;
	  Elf32_Word	e_flags;
	  Elf32_Half	e_ehsize;
	  Elf32_Half	e_phentsize;
	  Elf32_Half	e_phnum;
	  Elf32_Half	e_shentsize;
	  Elf32_Half	e_shnum;
	  Elf32_Half	e_shstrndx;
	} Elf32_Ehdr;

	typedef struct elf64_hdr {
	  unsigned char	e_ident[LINUX_EI_NIDENT];	/* ELF "magic number" */
	  Elf64_Half	e_type;
	  Elf64_Half	e_machine;
	  Elf64_Word	e_version;
	  Elf64_Addr	e_entry;		/* Entry point virtual address */
	  Elf64_Off		e_phoff;		/* Program header table file offset */
	  Elf64_Off		e_shoff;		/* Section header table file offset */
	  Elf64_Word	e_flags;
	  Elf64_Half	e_ehsize;
	  Elf64_Half	e_phentsize;
	  Elf64_Half	e_phnum;
	  Elf64_Half	e_shentsize;
	  Elf64_Half	e_shnum;
	  Elf64_Half	e_shstrndx;
	} Elf64_Ehdr;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

/* These constants define the permissions on sections in the program header, p_flags. */
#define LINUX_PF_R				0x4
#define LINUX_PF_W				0x2
#define LINUX_PF_X				0x1

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef struct elf32_phdr{
	  Elf32_Word	p_type;
	  Elf32_Off		p_offset;
	  Elf32_Addr	p_vaddr;
	  Elf32_Addr	p_paddr;
	  Elf32_Word	p_filesz;
	  Elf32_Word	p_memsz;
	  Elf32_Word	p_flags;
	  Elf32_Word	p_align;
	} Elf32_Phdr;

	typedef struct elf64_phdr {
	  Elf64_Word	p_type;
	  Elf64_Word	p_flags;
	  Elf64_Off		p_offset;		/* Segment file offset */
	  Elf64_Addr	p_vaddr;		/* Segment virtual address */
	  Elf64_Addr	p_paddr;		/* Segment physical address */
	  Elf64_Xword	p_filesz;		/* Segment size in file */
	  Elf64_Xword	p_memsz;		/* Segment size in memory */
	  Elf64_Xword	p_align;		/* Segment alignment, file & memory */
	} Elf64_Phdr;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

/* sh_type */
#define LINUX_SHT_NULL			0
#define LINUX_SHT_PROGBITS		1
#define LINUX_SHT_SYMTAB		2
#define LINUX_SHT_STRTAB		3
#define LINUX_SHT_RELA			4
#define LINUX_SHT_HASH			5
#define LINUX_SHT_DYNAMIC		6
#define LINUX_SHT_NOTE			7
#define LINUX_SHT_NOBITS		8
#define LINUX_SHT_REL			9
#define LINUX_SHT_SHLIB			10
#define LINUX_SHT_DYNSYM		11
#define LINUX_SHT_NUM			12
#define LINUX_SHT_LOPROC		0x70000000
#define LINUX_SHT_HIPROC		0x7fffffff
#define LINUX_SHT_LOUSER		0x80000000
#define LINUX_SHT_HIUSER		0xffffffff

/* sh_flags */
#define LINUX_SHF_WRITE			0x1
#define LINUX_SHF_ALLOC			0x2
#define LINUX_SHF_EXECINSTR		0x4
#define LINUX_SHF_MASKPROC		0xf0000000

/* special section indexes */
#define LINUX_SHN_UNDEF			0
#define LINUX_SHN_LORESERVE		0xff00
#define LINUX_SHN_LOPROC		0xff00
#define LINUX_SHN_HIPROC		0xff1f
#define LINUX_SHN_ABS			0xfff1
#define LINUX_SHN_COMMON		0xfff2
#define LINUX_SHN_HIRESERVE		0xffff

/* added to linux header */
#define LINUX_STN_UNDEF			0

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef struct elf32_shdr {
	  Elf32_Word	sh_name;
	  Elf32_Word	sh_type;
	  Elf32_Word	sh_flags;
	  Elf32_Addr	sh_addr;
	  Elf32_Off		sh_offset;
	  Elf32_Word	sh_size;
	  Elf32_Word	sh_link;
	  Elf32_Word	sh_info;
	  Elf32_Word	sh_addralign;
	  Elf32_Word	sh_entsize;
	} Elf32_Shdr;

	typedef struct elf64_shdr {
	  Elf64_Word	sh_name;		/* Section name, index in string tbl */
	  Elf64_Word	sh_type;		/* Type of section */
	  Elf64_Xword	sh_flags;		/* Miscellaneous section attributes */
	  Elf64_Addr	sh_addr;		/* Section virtual addr at execution */
	  Elf64_Off		sh_offset;		/* Section file offset */
	  Elf64_Xword	sh_size;		/* Size of section in bytes */
	  Elf64_Word	sh_link;		/* Index of another section */
	  Elf64_Word	sh_info;		/* Additional section information */
	  Elf64_Xword	sh_addralign;	/* Section alignment */
	  Elf64_Xword	sh_entsize;		/* Entry size if section holds table */
	} Elf64_Shdr;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

#define	LINUX_EI_MAG0			0		/* e_ident[] indexes */
#define	LINUX_EI_MAG1			1
#define	LINUX_EI_MAG2			2
#define	LINUX_EI_MAG3			3
#define	LINUX_EI_CLASS			4
#define	LINUX_EI_DATA			5
#define	LINUX_EI_VERSION		6
#define	LINUX_EI_OSABI			7
#define	LINUX_EI_PAD			8

#define	LINUX_ELFMAG0			0x7f		/* EI_MAG */
#define	LINUX_ELFMAG1			'E'
#define	LINUX_ELFMAG2			'L'
#define	LINUX_ELFMAG3			'F'
#define	LINUX_ELFMAG			"\177ELF"
#define	LINUX_SELFMAG			4

#define	LINUX_ELFCLASSNONE		0		/* EI_CLASS */
#define	LINUX_ELFCLASS32		1
#define	LINUX_ELFCLASS64		2
#define	LINUX_ELFCLASSNUM		3

#define LINUX_ELFDATANONE		0		/* e_ident[EI_DATA] */
#define LINUX_ELFDATA2LSB		1
#define LINUX_ELFDATA2MSB		2

#define LINUX_EV_NONE			0		/* e_version, EI_VERSION */
#define LINUX_EV_CURRENT		1
#define LINUX_EV_NUM			2

#define LINUX_ELFOSABI_NONE		0
#define LINUX_ELFOSABI_LINUX	3

#ifndef LINUX_ELF_OSABI
#define LINUX_ELF_OSABI LINUX_ELFOSABI_NONE
#endif

/*
 * Notes used in ET_CORE. Architectures export some of the arch register sets
 * using the corresponding note types via the PTRACE_GETREGSET and
 * PTRACE_SETREGSET requests.
 */
#define LINUX_NT_PRSTATUS		1
#define LINUX_NT_PRFPREG		2
#define LINUX_NT_PRPSINFO		3
#define LINUX_NT_TASKSTRUCT		4
#define LINUX_NT_AUXV			6
/*
 * Note to userspace developers: size of NT_SIGINFO note may increase
 * in the future to accomodate more fields, don't assume it is fixed!
 */
#define LINUX_NT_SIGINFO			0x53494749
#define LINUX_NT_FILE				0x46494c45
#define LINUX_NT_PRXFPREG			0x46e62b7f	/* copied from gdb5.1/include/elf/common.h */
#define LINUX_NT_PPC_VMX			0x100		/* PowerPC Altivec/VMX registers */
#define LINUX_NT_PPC_SPE			0x101		/* PowerPC SPE/EVR registers */
#define LINUX_NT_PPC_VSX			0x102		/* PowerPC VSX registers */
#define LINUX_NT_386_TLS			0x200		/* i386 TLS slots (struct user_desc) */
#define LINUX_NT_386_IOPERM			0x201		/* x86 io permission bitmap (1=deny) */
#define LINUX_NT_X86_XSTATE			0x202		/* x86 extended state using xsave */
#define LINUX_NT_S390_HIGH_GPRS		0x300		/* s390 upper register halves */
#define LINUX_NT_S390_TIMER			0x301		/* s390 timer register */
#define LINUX_NT_S390_TODCMP		0x302		/* s390 TOD clock comparator register */
#define LINUX_NT_S390_TODPREG		0x303		/* s390 TOD programmable register */
#define LINUX_NT_S390_CTRS			0x304		/* s390 control registers */
#define LINUX_NT_S390_PREFIX		0x305		/* s390 prefix register */
#define LINUX_NT_S390_LAST_BREAK	0x306		/* s390 breaking event address */
#define LINUX_NT_S390_SYSTEM_CALL	0x307		/* s390 system call restart data */
#define LINUX_NT_S390_TDB			0x308		/* s390 transaction diagnostic block */
#define LINUX_NT_ARM_VFP			0x400		/* ARM VFP/NEON registers */
#define LINUX_NT_ARM_TLS			0x401		/* ARM TLS register */
#define LINUX_NT_ARM_HW_BREAK		0x402		/* ARM hardware breakpoint registers */
#define LINUX_NT_ARM_HW_WATCH		0x403		/* ARM hardware watchpoint registers */
#define LINUX_NT_METAG_CBUF			0x500		/* Metag catch buffer registers */
#define LINUX_NT_METAG_RPIPE		0x501		/* Metag read pipeline state */
#define LINUX_NT_METAG_TLS			0x502		/* Metag TLS pointer */

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	/* Note header in a PT_NOTE section */
	typedef struct elf32_note {
	  Elf32_Word	n_namesz;	/* Name size */
	  Elf32_Word	n_descsz;	/* Content size */
	  Elf32_Word	n_type;		/* Content type */
	} Elf32_Nhdr;

	/* Note header in a PT_NOTE section */
	typedef struct elf64_note {
	  Elf64_Word	n_namesz;	/* Name size */
	  Elf64_Word	n_descsz;	/* Content size */
	  Elf64_Word	n_type;		/* Content type */
	} Elf64_Nhdr;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif	// __LINUX_ELF_H_