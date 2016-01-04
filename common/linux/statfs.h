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

#ifndef __LINUX_STATFS_H_
#define __LINUX_STATFS_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// include/linux/statfs.h
//-----------------------------------------------------------------------------

#define LINUX_ST_RDONLY			0x0001	/* mount read-only */
#define LINUX_ST_NOSUID			0x0002	/* ignore suid and sgid bits */
#define LINUX_ST_NODEV			0x0004	/* disallow access to device special files */
#define LINUX_ST_NOEXEC			0x0008	/* disallow program execution */
#define LINUX_ST_SYNCHRONOUS	0x0010	/* writes are synced at once */
#define LINUX_ST_VALID			0x0020	/* f_flags support is implemented */
#define LINUX_ST_MANDLOCK		0x0040	/* allow mandatory locks on an FS */
										/* 0x0080 used for ST_WRITE in glibc */
										/* 0x0100 used for ST_APPEND in glibc */
										/* 0x0200 used for ST_IMMUTABLE in glibc */
#define LINUX_ST_NOATIME		0x0400	/* do not update access times */
#define LINUX_ST_NODIRATIME		0x0800	/* do not update directory access times */
#define LINUX_ST_RELATIME		0x1000	/* update atime relative to mtime/ctime */

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