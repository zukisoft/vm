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

#ifndef __LINUX_MMAN_H_
#define __LINUX_MMAN_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/mman-common.h
//-----------------------------------------------------------------------------

#define LINUX_PROT_READ				0x1				/* page can be read */
#define LINUX_PROT_WRITE			0x2				/* page can be written */
#define LINUX_PROT_EXEC				0x4				/* page can be executed */
#define LINUX_PROT_SEM				0x8				/* page may be used for atomic ops */
#define LINUX_PROT_NONE				0x0				/* page can not be accessed */
#define LINUX_PROT_GROWSDOWN		0x01000000		/* mprotect flag: extend change to start of growsdown vma */
#define LINUX_PROT_GROWSUP			0x02000000		/* mprotect flag: extend change to end of growsup vma */

#define LINUX_MAP_SHARED			0x01			/* Share changes */
#define LINUX_MAP_PRIVATE			0x02			/* Changes are private */
#define LINUX_MAP_TYPE				0x0F			/* Mask for type of mapping */
#define LINUX_MAP_FIXED				0x10			/* Interpret addr exactly */
#define LINUX_MAP_ANONYMOUS			0x20			/* don't use a file */
#define LINUX_MAP_UNINITIALIZED		0x4000000		/* For anonymous mmap, memory could be uninitialized */

#define LINUX_MS_ASYNC				1				/* sync memory asynchronously */
#define LINUX_MS_INVALIDATE			2				/* invalidate the caches */
#define LINUX_MS_SYNC				4				/* synchronous memory sync */

#define LINUX_MADV_NORMAL			0				/* no further special treatment */
#define LINUX_MADV_RANDOM			1				/* expect random page references */
#define LINUX_MADV_SEQUENTIAL		2				/* expect sequential page references */
#define LINUX_MADV_WILLNEED			3				/* will need these pages */
#define LINUX_MADV_DONTNEED			4				/* don't need these pages */

#define LINUX_MADV_REMOVE			9				/* remove these pages & resources */
#define LINUX_MADV_DONTFORK			10				/* don't inherit across fork */
#define LINUX_MADV_DOFORK			11				/* do inherit across fork */
#define LINUX_MADV_HWPOISON			100				/* poison a page for testing */
#define LINUX_MADV_SOFT_OFFLINE		101				/* soft offline page for testing */

#define LINUX_MADV_MERGEABLE		12				/* KSM may merge identical pages */
#define LINUX_MADV_UNMERGEABLE		13				/* KSM may not merge identical pages */
#define LINUX_MADV_HUGEPAGE			14				/* Worth backing with hugepages */
#define LINUX_MADV_NOHUGEPAGE		15				/* Not worth backing with hugepages */
#define LINUX_MADV_DONTDUMP			16				/* Explicity exclude from the core dump, overrides the coredump filter bits */
#define LINUX_MADV_DODUMP			17				/* Clear the MADV_NODUMP flag */

#define LINUX_MAP_FILE				0
#define LINUX_MAP_HUGE_SHIFT		26
#define LINUX_MAP_HUGE_MASK			0x3F

#if !defined(__midl) && defined(__cplusplus)

__declspec(selectany)
const void* LINUX_MAP_FAILED		= reinterpret_cast<void*>(-1);

#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/mman.h
//-----------------------------------------------------------------------------

#define LINUX_MAP_GROWSDOWN			0x0100			/* stack-like segment */
#define LINUX_MAP_DENYWRITE			0x0800			/* ETXTBSY */
#define LINUX_MAP_EXECUTABLE		0x1000			/* mark it as an executable */
#define LINUX_MAP_LOCKED			0x2000			/* pages are locked */
#define LINUX_MAP_NORESERVE			0x4000			/* don't check for reservations */
#define LINUX_MAP_POPULATE			0x8000			/* populate (prefault) pagetables */
#define LINUX_MAP_NONBLOCK			0x10000			/* do not block on IO */
#define LINUX_MAP_STACK				0x20000			/* give out an address that is best suited for process/thread stacks */
#define LINUX_MAP_HUGETLB			0x40000			/* create a huge page mapping */

#define LINUX_MCL_CURRENT			1				/* lock all current mappings */
#define LINUX_MCL_FUTURE			2				/* lock all future mappings */

//-----------------------------------------------------------------------------
// arch/x86/include/uapi/asm/mman.h
//-----------------------------------------------------------------------------

#define LINUX_MAP_32BIT				0x40			/* only give out 32bit addresses */

#define LINUX_MAP_HUGE_2MB			(21 << LINUX_MAP_HUGE_SHIFT)
#define LINUX_MAP_HUGE_1GB			(30 << LINUX_MAP_HUGE_SHIFT)

//-----------------------------------------------------------------------------

#endif		// __LINUX_MMAN_H_