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

#ifndef __LINUX_FS_H_
#define __LINUX_FS_H_
#pragma once

//-----------------------------------------------------------------------------
// include/linux/fs.h
//-----------------------------------------------------------------------------

#define LINUX_MAY_EXEC			0x00000001
#define LINUX_MAY_WRITE			0x00000002
#define LINUX_MAY_READ			0x00000004
#define LINUX_MAY_APPEND		0x00000008
#define LINUX_MAY_ACCESS		0x00000010
#define LINUX_MAY_OPEN			0x00000020
#define LINUX_MAY_CHDIR			0x00000040

//-----------------------------------------------------------------------------
// include/uapi/linux/fs.h
//-----------------------------------------------------------------------------

#define LINUX_SEEK_SET			0		/* seek relative to beginning of file */
#define LINUX_SEEK_CUR			1		/* seek relative to current file position */
#define LINUX_SEEK_END			2		/* seek relative to end of file */
#define LINUX_SEEK_DATA			3		/* seek to the next data */
#define LINUX_SEEK_HOLE			4		/* seek to the next hole */
#define LINUX_SEEK_MAX			LINUX_SEEK_HOLE

#define LINUX_MS_RDONLY			1		/* Mount read-only */
#define LINUX_MS_NOSUID			2		/* Ignore suid and sgid bits */
#define LINUX_MS_NODEV			4		/* Disallow access to device special files */
#define LINUX_MS_NOEXEC			8		/* Disallow program execution */
#define LINUX_MS_SYNCHRONOUS	16		/* Writes are synced at once */
#define LINUX_MS_REMOUNT		32		/* Alter flags of a mounted FS */
#define LINUX_MS_MANDLOCK		64		/* Allow mandatory locks on an FS */
#define LINUX_MS_DIRSYNC		128		/* Directory modifications are synchronous */
#define LINUX_MS_NOATIME		1024	/* Do not update access times. */
#define LINUX_MS_NODIRATIME		2048	/* Do not update directory access times */
#define LINUX_MS_BIND			4096
#define LINUX_MS_MOVE			8192
#define LINUX_MS_REC			16384
#define LINUX_MS_SILENT			32768
#define LINUX_MS_POSIXACL		(1<<16)	/* VFS does not apply the umask */
#define LINUX_MS_UNBINDABLE		(1<<17)	/* change to unbindable */
#define LINUX_MS_PRIVATE		(1<<18)	/* change to private */
#define LINUX_MS_SLAVE			(1<<19)	/* change to slave */
#define LINUX_MS_SHARED			(1<<20)	/* change to shared */
#define LINUX_MS_RELATIME		(1<<21)	/* Update atime relative to mtime/ctime. */
#define LINUX_MS_KERNMOUNT		(1<<22)	/* this is a kern_mount call */
#define LINUX_MS_I_VERSION		(1<<23)	/* Update inode I_version field */
#define LINUX_MS_STRICTATIME	(1<<24)	/* Always perform atime updates */
#define LINUX_MS_LAZYTIME		(1<<25) /* Update the on-disk [acm]times lazily */

// MS_RMT_MASK - Mount options that can be changed during an MS_REMOUNT operation
#define LINUX_MS_RMT_MASK     (LINUX_MS_RDONLY | LINUX_MS_SYNCHRONOUS | LINUX_MS_MANDLOCK | LINUX_MS_I_VERSION | LINUX_MS_LAZYTIME)

// MS_PERMOUNT_MASK - Mount options that are per-mount rather than applicable to
// an entire file system (see mount(2) man page)
#define LINUX_MS_PERMOUNT_MASK	(LINUX_MS_NODEV | LINUX_MS_NOEXEC | LINUX_MS_NOSUID | LINUX_MS_NOATIME | LINUX_MS_NODIRATIME | LINUX_MS_RELATIME)

//-----------------------------------------------------------------------------

#endif		// __LINUX_FS_H_