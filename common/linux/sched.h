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

#ifndef __LINUX_SCHED_H_
#define __LINUX_SCHED_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/sched.h
//-----------------------------------------------------------------------------

#define LINUX_CSIGNAL					0x000000FF	/* signal mask to be sent at exit */
#define LINUX_CLONE_VM					0x00000100	/* set if VM shared between processes */
#define LINUX_CLONE_FS					0x00000200	/* set if fs info shared between processes */
#define LINUX_CLONE_FILES				0x00000400	/* set if open files shared between processes */
#define LINUX_CLONE_SIGHAND				0x00000800	/* set if signal handlers and blocked signals shared */
#define LINUX_CLONE_PTRACE				0x00002000	/* set if we want to let tracing continue on the child too */
#define LINUX_CLONE_VFORK				0x00004000	/* set if the parent wants the child to wake it up on mm_release */
#define LINUX_CLONE_PARENT				0x00008000	/* set if we want to have the same parent as the cloner */
#define LINUX_CLONE_THREAD				0x00010000	/* Same thread group? */
#define LINUX_CLONE_NEWNS				0x00020000	/* New namespace group? */
#define LINUX_CLONE_SYSVSEM				0x00040000	/* share system V SEM_UNDO semantics */
#define LINUX_CLONE_SETTLS				0x00080000	/* create a new TLS for the child */
#define LINUX_CLONE_PARENT_SETTID		0x00100000	/* set the TID in the parent */
#define LINUX_CLONE_CHILD_CLEARTID		0x00200000	/* clear the TID in the child */
#define LINUX_CLONE_DETACHED			0x00400000	/* Unused, ignored */
#define LINUX_CLONE_UNTRACED			0x00800000	/* set if the tracing process can't force CLONE_PTRACE on this clone */
#define LINUX_CLONE_CHILD_SETTID		0x01000000	/* set the TID in the child */
#define LINUX_CLONE_NEWUTS				0x04000000	/* New utsname group? */
#define LINUX_CLONE_NEWIPC				0x08000000	/* New ipcs */
#define LINUX_CLONE_NEWUSER				0x10000000	/* New user namespace */
#define LINUX_CLONE_NEWPID				0x20000000	/* New pid namespace */
#define LINUX_CLONE_NEWNET				0x40000000	/* New network namespace */
#define LINUX_CLONE_IO					0x80000000	/* Clone io context */

#define LINUX_SCHED_NORMAL				0
#define LINUX_SCHED_FIFO				1
#define LINUX_SCHED_RR					2
#define LINUX_SCHED_BATCH				3
#define LINUX_SCHED_IDLE				5
#define LINUX_SCHED_DEADLINE			6

#define LINUX_SCHED_RESET_ON_FORK		0x40000000
#define LINUX_SCHED_FLAG_RESET_ON_FORK	0x01

//-----------------------------------------------------------------------------

#endif		// __LINUX_SCHED_H_
