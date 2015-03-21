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

#ifndef __LINUX_WAIT_H_
#define __LINUX_WAIT_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/wait.h
//-----------------------------------------------------------------------------

#define LINUX_WNOHANG			0x00000001
#define LINUX_WUNTRACED			0x00000002
#define LINUX_WSTOPPED			LINUX_WUNTRACED
#define LINUX_WEXITED			0x00000004
#define LINUX_WCONTINUED		0x00000008
#define LINUX_WNOWAIT			0x01000000		/* Don't reap, just poll status.  */

#define LINUX__WNOTHREAD		0x20000000		/* Don't wait on children of other threads in this group */
#define LINUX__WALL				0x40000000		/* Wait on all children, regardless of type */
#define LINUX__WCLONE			0x80000000		/* Wait only on non-SIGCHLD children */

#define LINUX_P_ALL				0
#define LINUX_P_PID				1
#define LINUX_P_PGID			2

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	// RUNNING
	//
	// Process/thread status code when running
	const int RUNNING = 0xFFFF;

	// STOPPED
	//
	// Process/thread status code when suspended
	const int STOPPED = 0x007F;

	// Constructs a waitable status/exit code from component values
	//
	inline int MakeExitCode(int status, int signal, bool coredump)
	{
		// Create the packed status code for the task, which is a 16-bit value that
		// contains the actual status code in the upper 8 bits and flags in the lower 8
		return ((status & 0xFF) << 8) | (signal & 0xFF) | (coredump ? 0x80 : 0);
	}

	// Constructs a waitable status/exit code from component values
	//
	inline int MakeExitCode(int status, int signal)
	{
		return MakeExitCode(status, signal, false);
	}

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_WAIT_H_