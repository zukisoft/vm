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

#ifndef __LINUX_STAT_H_
#define __LINUX_STAT_H_
#pragma once

#include "types.h"
#include "time.h"

//-----------------------------------------------------------------------------
// include/uapi/linux/stat.h
//-----------------------------------------------------------------------------

// note: all these constants are in octal
//
#define LINUX_S_IFMT		0170000
#define LINUX_S_IFSOCK		0140000
#define LINUX_S_IFLNK		0120000
#define LINUX_S_IFREG		0100000
#define LINUX_S_IFBLK		0060000
#define LINUX_S_IFDIR		0040000
#define LINUX_S_IFCHR		0020000
#define LINUX_S_IFIFO		0010000
#define LINUX_S_ISUID		0004000
#define LINUX_S_ISGID		0002000
#define LINUX_S_ISVTX		0001000

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	inline bool S_ISLNK(mode_t mode)	{ return ((mode & LINUX_S_IFMT) == LINUX_S_IFLNK); }
	inline bool S_ISREG(mode_t mode)	{ return ((mode & LINUX_S_IFMT) == LINUX_S_IFREG); }
	inline bool S_ISDIR(mode_t mode)	{ return ((mode & LINUX_S_IFMT) == LINUX_S_IFDIR); }
	inline bool S_ISCHR(mode_t mode)	{ return ((mode & LINUX_S_IFMT) == LINUX_S_IFCHR); }
	inline bool S_ISBLK(mode_t mode)	{ return ((mode & LINUX_S_IFMT) == LINUX_S_IFBLK); }
	inline bool S_ISFIFO(mode_t mode)	{ return ((mode & LINUX_S_IFMT) == LINUX_S_IFIFO); }
	inline bool S_ISSOCK(mode_t mode)	{ return ((mode & LINUX_S_IFMT) == LINUX_S_IFSOCK); }

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

#define LINUX_S_IRWXU		0700
#define LINUX_S_IRUSR		0400
#define LINUX_S_IWUSR		0200
#define LINUX_S_IXUSR		0100

#define LINUX_S_IRWXG		0070
#define LINUX_S_IRGRP		0040
#define LINUX_S_IWGRP		0020
#define LINUX_S_IXGRP		0010

#define LINUX_S_IRWXO		0007
#define LINUX_S_IROTH		0004
#define LINUX_S_IWOTH		0002
#define LINUX_S_IXOTH		0001

//-----------------------------------------------------------------------------
// include/linux/stat.h
//-----------------------------------------------------------------------------

#define LINUX_S_IRWXUGO		(LINUX_S_IRWXU | LINUX_S_IRWXG | LINUX_S_IRWXO)
#define LINUX_S_IALLUGO		(LINUX_S_ISUID | LINUX_S_ISGID | LINUX_S_ISVTX | LINUX_S_IRWXUGO)
#define LINUX_S_IRUGO		(LINUX_S_IRUSR | LINUX_S_IRGRP | LINUX_S_IROTH)
#define LINUX_S_IWUGO		(LINUX_S_IWUSR | LINUX_S_IWGRP | LINUX_S_IWOTH)
#define LINUX_S_IXUGO		(LINUX_S_IXUSR | LINUX_S_IXGRP | LINUX_S_IXOTH)

//-----------------------------------------------------------------------------
// arch/x86/include/uapi/asm/stat.h
//-----------------------------------------------------------------------------

#pragma pack(push, 1)

// size = 30 (x86)
//
// Used only with 32-bit stat() and family
typedef struct {

	uint16_t		st_dev;
	uint16_t		st_ino;
	uint16_t		st_mode;
	uint16_t		st_nlink;
	uint16_t		st_uid;
	uint16_t		st_gid;
	uint16_t		st_rdev;
	uint32_t		st_size;
	uint32_t		st_atime;
	uint32_t		st_mtime;
	uint32_t		st_ctime;

} linux_oldstat;

// size = 64 (x86)
//
// Used with 32-bit newstat() and family
typedef struct {

	uint32_t			st_dev;
	uint32_t			st_ino;
	uint16_t			st_mode;
	uint16_t			st_nlink;
	uint16_t			st_uid;
	uint16_t			st_gid;
	uint32_t			st_rdev;
	uint32_t			st_size;
	uint32_t			st_blksize;
	uint32_t			st_blocks;
	linux_timespec32	st_atime;
	linux_timespec32	st_mtime;
	linux_timespec32	st_ctime;
	uint32_t			__unused4;
	uint32_t			__unused5;

} linux_stat32;

// size = 144 (x64)
//
// Used with 64-bit stat() and family
typedef struct {

	uint64_t			st_dev;
	uint64_t			st_ino;
	uint64_t			st_nlink;
	uint32_t			st_mode;
	uint32_t			st_uid;
	uint32_t			st_gid;
	uint32_t			__pad0;
	uint64_t			st_rdev;
	int64_t				st_size;
	int64_t				st_blksize;
	int64_t				st_blocks;
	linux_timespec64	st_atime;
	linux_timespec64	st_mtime;
	linux_timespec64	st_ctime;	
	int64_t				__unused[3];

} linux_stat64;

// size = 96 (x86)
//
// Used with 32-bit stat64() and family
typedef struct {

	uint64_t			st_dev;
	uint8_t				__pad0[4];
	uint32_t			__st_ino;
	uint32_t			st_mode;
	uint32_t			st_nlink;
	uint32_t			st_uid;
	uint32_t			st_gid;
	uint64_t			st_rdev;
	uint8_t				__pad3[4];
	int64_t				st_size;
	uint32_t			st_blksize;
	uint64_t			st_blocks;
	linux_timespec32	st_atime;
	linux_timespec32	st_mtime;
	linux_timespec32	st_ctime;
	uint64_t			st_ino;

} linux_stat3264;

#pragma pack(pop)

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_oldstat			oldstat;
	typedef linux_stat32			stat32;
	typedef linux_stat3264			stat3264;
	typedef linux_stat64			stat64;

	// stat is the version of the structure used by the Virtual Machine
	typedef linux_stat64			stat;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_STAT_H_