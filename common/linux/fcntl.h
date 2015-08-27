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

#ifndef __LINUX_FCNTL_H_
#define __LINUX_FCNTL_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/fcntl.h
//-----------------------------------------------------------------------------

// note: following constants are in octal
//
#define LINUX_O_RDONLY				00000000
#define LINUX_O_WRONLY				00000001
#define LINUX_O_RDWR				00000002
#define LINUX_O_ACCMODE				00000003
#define LINUX_O_CREAT				00000100	/* not fcntl */
#define LINUX_O_EXCL				00000200	/* not fcntl */
#define LINUX_O_NOCTTY				00000400	/* not fcntl */
#define LINUX_O_TRUNC				00001000	/* not fcntl */
#define LINUX_O_APPEND				00002000
#define LINUX_O_NONBLOCK			00004000
#define LINUX_O_DSYNC				00010000	/* used to be LINUX_O_SYNC, see below */
#define LINUX_FASYNC				00020000	/* fcntl, for BSD compatibility */
#define LINUX_O_DIRECT				00040000	/* direct disk access hint */
#define LINUX_O_LARGEFILE			00100000
#define LINUX_O_DIRECTORY			00200000	/* must be a directory */
#define LINUX_O_NOFOLLOW			00400000	/* don't follow links */
#define LINUX_O_NOATIME				01000000
#define LINUX_O_CLOEXEC				02000000	/* set close_on_exec */
#define LINUX___O_SYNC				04000000
#define LINUX_O_SYNC				(LINUX___O_SYNC | LINUX_O_DSYNC)
#define LINUX_O_PATH				010000000
#define LINUX___O_TMPFILE			020000000
#define LINUX_O_TMPFILE				(LINUX_O_DIRECTORY | LINUX___O_TMPFILE)

#define LINUX_F_DUPFD				0			/* dup */
#define LINUX_F_GETFD				1			/* get close_on_exec */
#define LINUX_F_SETFD				2			/* set/clear close_on_exec */
#define LINUX_F_GETFL				3			/* get file->f_flags */
#define LINUX_F_SETFL				4			/* set file->f_flags */
#define LINUX_F_GETLK				5
#define LINUX_F_SETLK				6
#define LINUX_F_SETLKW				7
#define LINUX_F_SETOWN				8			/* for sockets. */
#define LINUX_F_GETOWN				9			/* for sockets. */
#define LINUX_F_SETSIG				10			/* for sockets. */
#define LINUX_F_GETSIG				11			/* for sockets. */
#define LINUX_F_GETLK64				12			/*  using 'struct flock64' */
#define LINUX_F_SETLK64				13
#define LINUX_F_SETLKW64			14
#define LINUX_F_SETOWN_EX			15
#define LINUX_F_GETOWN_EX			16
#define LINUX_F_GETOWNER_UIDS		17

#define LINUX_F_LINUX_SPECIFIC_BASE	1024

#define LINUX_FD_CLOEXEC			1			/* F_[GET|SET]FL flag */

//-----------------------------------------------------------------------------
// include/uapi/linux/fcntl.h
//-----------------------------------------------------------------------------

#define LINUX_F_SETLEASE			(LINUX_F_LINUX_SPECIFIC_BASE + 0)
#define LINUX_F_GETLEASE			(LINUX_F_LINUX_SPECIFIC_BASE + 1)
#define LINUX_F_NOTIFY				(LINUX_F_LINUX_SPECIFIC_BASE + 2)
// no + 3 constant
// no + 4 constant
#define LINUX_F_CANCELLK			(LINUX_F_LINUX_SPECIFIC_BASE + 5)
#define LINUX_F_DUPFD_CLOEXEC		(LINUX_F_LINUX_SPECIFIC_BASE + 6)
#define LINUX_F_SETPIPE_SZ			(LINUX_F_LINUX_SPECIFIC_BASE + 7)
#define LINUX_F_GETPIPE_SZ			(LINUX_F_LINUX_SPECIFIC_BASE + 8)
#define LINUX_F_ADD_SEALS			(LINUX_F_LINUX_SPECIFIC_BASE + 9)
#define LINUX_F_GET_SEALS			(LINUX_F_LINUX_SPECIFIC_BASE + 10)

#define LINUX_AT_FDCWD				 -100		/* Special value used to indicate current working directory. */
#define LINUX_AT_SYMLINK_NOFOLLOW	0x100		/* Do not follow symbolic links.  */
#define LINUX_AT_REMOVEDIR			0x200		/* Remove directory instead of unlinking file.  */
#define LINUX_AT_SYMLINK_FOLLOW		0x400		/* Follow symbolic links. */
#define LINUX_AT_NO_AUTOMOUNT		0x800		/* Suppress terminal automount traversal */
#define LINUX_AT_EMPTY_PATH			0x1000		/* Allow empty relative pathname */

//-----------------------------------------------------------------------------

#endif		// __LINUX_FCNTL_H_