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

//-----------------------------------------------------------------------------

#endif		// __LINUX_FCNTL_H_