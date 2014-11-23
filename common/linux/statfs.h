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

#ifndef __LINUX_STATFS_H_
#define __LINUX_STATFS_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/statfs.h
//-----------------------------------------------------------------------------

#pragma pack(push, 1)

// size = 64 (x86)
//
// Used only with 32-bit statfs() and family
typedef struct {

	uint32_t		f_type;
	uint32_t		f_bsize;
	uint32_t		f_blocks;
	uint32_t		f_bfree;
	uint32_t		f_bavail;
	uint32_t		f_files;
	uint32_t		f_ffree;
	__kernel_fsid_t	f_fsid;
	uint32_t		f_namelen;
	uint32_t		f_frsize;
	uint32_t		f_flags;
	uint32_t		f_spare[4];

} linux_statfs32;

// size = 84 (x86)
//
// Used only with 32-bit statfs64() and family
typedef struct {

	int32_t				f_type;
	int32_t				f_bsize;
	uint64_t			f_blocks;
	uint64_t			f_bfree;
	uint64_t			f_bavail;
	uint64_t			f_files;
	uint64_t			f_ffree;
	__kernel_fsid_t		f_fsid;
	int32_t				f_namelen;
	int32_t				f_frsize;
	int32_t				f_flags;
	int32_t				f_spare[4];

} linux_statfs3264;

// size = 120 (x64)
//
// Used only with 64-bit statfs() and family
typedef struct {

	int64_t				f_type;
	int64_t				f_bsize;
	uint64_t			f_blocks;
	uint64_t			f_bfree;
	uint64_t			f_bavail;
	uint64_t			f_files;
	uint64_t			f_ffree;
	__kernel_fsid_t		f_fsid;
	int64_t				f_namelen;
	int64_t				f_frsize;
	int64_t				f_flags;
	int64_t				f_spare[4];

} linux_statfs64;

#pragma pack(pop)

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_statfs32			statfs32;
	typedef linux_statfs3264		statfs3264;
	typedef linux_statfs64			statfs64;

	// stat is the version of the structure used by the Virtual Machine
	typedef linux_statfs64			statfs;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_STATFS_H_