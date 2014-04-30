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

#ifndef __LINUX_STAT_H_
#define __LINUX_STAT_H_
#pragma once

#include "types.h"

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

#endif		// __LINUX_STAT_H_