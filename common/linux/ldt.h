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

#ifndef __LINUX_LDT_H_
#define __LINUX_LDT_H_
#pragma once

//-----------------------------------------------------------------------------
// arch/x86/include/uapi/asm/ldt.h
//-----------------------------------------------------------------------------

#define LINUX_LDT_ENTRIES			8192	/* Maximum number of LDT entries supported. */

typedef struct {

	uint32_t	entry_number;
	uint32_t	base_addr;
	uint32_t	limit;
	uint32_t	flags;
	
} linux_user_desc32;

typedef struct {

	uint32_t	entry_number;
	uint64_t	base_addr;
	uint32_t	limit;
	uint32_t	flags:1;
	
} linux_user_desc64;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef struct {

		uint32_t	entry_number;
		uint32_t	base_addr;
		uint32_t	limit;
		uint32_t	seg_32bit:1;
		uint32_t	contents:2;
		uint32_t	read_exec_only:1;
		uint32_t	limit_in_pages:1;
		uint32_t	seg_not_present:1;
		uint32_t	useable:1;
	
	} user_desc32;

	typedef struct {

		uint32_t	entry_number;
		uint64_t	base_addr;
		uint32_t	limit;
		uint32_t	seg_32bit:1;
		uint32_t	contents:2;
		uint32_t	read_exec_only:1;
		uint32_t	limit_in_pages:1;
		uint32_t	seg_not_present:1;
		uint32_t	useable:1;
	
	} user_desc64;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

#define LINUX_MODIFY_LDT_CONTENTS_DATA		0
#define LINUX_MODIFY_LDT_CONTENTS_STACK		1
#define LINUX_MODIFY_LDT_CONTENTS_CODE		2

//-----------------------------------------------------------------------------

#endif		// __LINUX_LDT_H_