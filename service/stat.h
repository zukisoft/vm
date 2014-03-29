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

#ifndef __STAT_H_
#define __STAT_H_
#pragma once

#pragma warning(push, 4)

// TODO: These constants will need to go into the .IDL file
// for the COM (or RPC?) interface

#define S_IFMT		0x00170000
#define S_IFSOCK	0x00140000
#define S_IFLNK		0x00120000
#define S_IFREG		0x00100000
#define S_IFBLK		0x00060000
#define S_IFDIR		0x00040000
#define S_IFCHR		0x00020000
#define S_IFIFO		0x00010000
#define S_ISUID		0x00004000
#define S_ISGID		0x00002000
#define S_ISVTX		0x00001000

#define S_ISLNK(m)	(((m) & S_IFMT) == S_IFLNK)
#define S_ISREG(m)	(((m) & S_IFMT) == S_IFREG)
#define S_ISDIR(m)	(((m) & S_IFMT) == S_IFDIR)
#define S_ISCHR(m)	(((m) & S_IFMT) == S_IFCHR)
#define S_ISBLK(m)	(((m) & S_IFMT) == S_IFBLK)
#define S_ISFIFO(m)	(((m) & S_IFMT) == S_IFIFO)
#define S_ISSOCK(m)	(((m) & S_IFMT) == S_IFSOCK)

#define S_IRWXU		0x0700
#define S_IRUSR		0x0400
#define S_IWUSR		0x0200
#define S_IXUSR		0x0100

#define S_IRWXG		0x0070
#define S_IRGRP		0x0040
#define S_IWGRP		0x0020
#define S_IXGRP		0x0010

#define S_IRWXO		0x0007
#define S_IROTH		0x0004
#define S_IWOTH		0x0002
#define S_IXOTH		0x0001

#pragma warning(pop)

#endif /* _UAPI_LINUX_STAT_H */