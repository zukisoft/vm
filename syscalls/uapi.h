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

#ifndef __UAPI_H_
#define __UAPI_H_

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// mman-common.h
//-----------------------------------------------------------------------------

#define PROT_READ				0x1				/* page can be read */
#define PROT_WRITE				0x2				/* page can be written */
#define PROT_EXEC				0x4				/* page can be executed */
#define PROT_SEM				0x8				/* page may be used for atomic ops */
#define PROT_NONE				0x0				/* page can not be accessed */
#define PROT_GROWSDOWN			0x01000000		/* mprotect flag: extend change to start of growsdown vma */
#define PROT_GROWSUP			0x02000000		/* mprotect flag: extend change to end of growsup vma */

#define MAP_SHARED				0x01			/* Share changes */
#define MAP_PRIVATE				0x02			/* Changes are private */
#define MAP_TYPE				0x0f			/* Mask for type of mapping */
#define MAP_FIXED				0x10			/* Interpret addr exactly */
#define MAP_ANONYMOUS			0x20			/* don't use a file */
#define MAP_UNINITIALIZED		0x4000000		/* For anonymous mmap, memory could be uninitialized */

#define MS_ASYNC				1				/* sync memory asynchronously */
#define MS_INVALIDATE			2				/* invalidate the caches */
#define MS_SYNC					4				/* synchronous memory sync */

#define MADV_NORMAL				0				/* no further special treatment */
#define MADV_RANDOM				1				/* expect random page references */
#define MADV_SEQUENTIAL			2				/* expect sequential page references */
#define MADV_WILLNEED			3				/* will need these pages */
#define MADV_DONTNEED			4				/* don't need these pages */

#define MADV_REMOVE				9				/* remove these pages & resources */
#define MADV_DONTFORK			10				/* don't inherit across fork */
#define MADV_DOFORK				11				/* do inherit across fork */
#define MADV_HWPOISON			100				/* poison a page for testing */
#define MADV_SOFT_OFFLINE		101				/* soft offline page for testing */

#define MADV_MERGEABLE			12				/* KSM may merge identical pages */
#define MADV_UNMERGEABLE		13				/* KSM may not merge identical pages */
#define MADV_HUGEPAGE			14				/* Worth backing with hugepages */
#define MADV_NOHUGEPAGE			15				/* Not worth backing with hugepages */

#define MADV_DONTDUMP			16				/* Explicity exclude from the core dump, overrides the coredump filter bits */
#define MADV_DODUMP				17				/* Clear the MADV_NODUMP flag */

#define MAP_FILE				0				/* Compatibility flag */
#define MAP_HUGE_SHIFT			26
#define MAP_HUGE_MASK			0x3F

#define MAP_FAILED				((void*)-1)

//-----------------------------------------------------------------------------
// mman.h
//-----------------------------------------------------------------------------

#define MAP_GROWSDOWN			0x0100			/* stack-like segment */
#define MAP_DENYWRITE			0x0800			/* ETXTBSY */
#define MAP_EXECUTABLE			0x1000			/* mark it as an executable */
#define MAP_LOCKED				0x2000			/* pages are locked */
#define MAP_NORESERVE			0x4000			/* don't check for reservations */
#define MAP_POPULATE			0x8000			/* populate (prefault) pagetables */
#define MAP_NONBLOCK			0x10000			/* do not block on IO */
#define MAP_STACK				0x20000			/* give out an address that is best suited for process/thread stacks */
#define MAP_HUGETLB				0x40000			/* create a huge page mapping */

#define MCL_CURRENT				1				/* lock all current mappings */
#define MCL_FUTURE				2				/* lock all future mappings */

//-----------------------------------------------------------------------------
// arch/x86/include/uapi/asm/mman.h
//-----------------------------------------------------------------------------

#define MAP_32BIT				0x40			/* only give out 32bit addresses */

#define MAP_HUGE_2MB			(21 << MAP_HUGE_SHIFT)
#define MAP_HUGE_1GB			(30 << MAP_HUGE_SHIFT)

#define MAP_UNINITIALIZED		0x4000000

// Converts Linux PROT_XXX flags into Windows PAGE_XXX flags
//
inline static uint32_t ProtToPageFlags(uint32_t prot)
{
	switch(prot & (PROT_EXEC | PROT_WRITE | PROT_READ)) {

		case PROT_EXEC:								return PAGE_EXECUTE;
		case PROT_WRITE :							return PAGE_READWRITE;
		case PROT_READ :							return PAGE_READONLY;
		case PROT_EXEC | PROT_WRITE :				return PAGE_EXECUTE_READWRITE;
		case PROT_EXEC | PROT_READ :				return PAGE_EXECUTE_READ;
		case PROT_WRITE | PROT_READ :				return PAGE_READWRITE;
		case PROT_EXEC | PROT_WRITE | PROT_READ :	return PAGE_EXECUTE_READWRITE;
	}

	return PAGE_NOACCESS;
}

// Converts Linux PROT_XXX flags into Windows FILE_MAP_XXX flags
//
inline static uint32_t ProtToFileMapFlags(uint32_t prot)
{
	uint32_t flags = 0;
	
	if(prot & PROT_READ) flags |= FILE_MAP_READ;
	if(prot & PROT_WRITE) flags |= FILE_MAP_WRITE;
	if(prot & PROT_EXEC) flags |= FILE_MAP_EXECUTE;

	return flags;
}


typedef int pid_t;


struct user_desc {
	uint32_t entry_number;
	uint32_t base_addr;
	uint32_t limit;
	uint32_t seg_32bit:1;
	uint32_t contents:2;
	uint32_t read_exec_only:1;
	uint32_t limit_in_pages:1;
	uint32_t seg_not_present:1;
	uint32_t useable:1;
#ifdef _M_X64
	uint32_t lm:1;
#endif
};


//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif		// __UAPI_H_
