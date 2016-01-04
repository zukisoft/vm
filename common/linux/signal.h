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

#ifndef __LINUX_SIGNAL_H_
#define __LINUX_SIGNAL_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/signal-defs.h
//-----------------------------------------------------------------------------

#define LINUX_SIG_BLOCK			0		/* for blocking signals */
#define LINUX_SIG_UNBLOCK		1		/* for unblocking signals */
#define LINUX_SIG_SETMASK		2		/* for setting the signal mask */

typedef void linux__signalfn_t(int);
typedef linux__signalfn_t *linux__sighandler_t;

typedef void linux__restorefn_t(void);
typedef linux__restorefn_t *linux__sigrestore_t;

#define LINUX_SIG_DFL			((linux__sighandler_t)0)		/* default signal handling */
#define LINUX_SIG_IGN			((linux__sighandler_t)1)		/* ignore signal */
#define LINUX_SIG_ERR			((linux__sighandler_t)-1)		/* error return from signal */

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux__sighandler_t		sighandler_t;
	typedef linux__sigrestore_t		sigrestore_t;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/signal.h
//-----------------------------------------------------------------------------

#define LINUX__NSIG				64

#define LINUX_SIGHUP			1
#define LINUX_SIGINT			2
#define LINUX_SIGQUIT			3
#define LINUX_SIGILL			4
#define LINUX_SIGTRAP			5
#define LINUX_SIGABRT			6
#define LINUX_SIGIOT			6
#define LINUX_SIGBUS			7
#define LINUX_SIGFPE			8
#define LINUX_SIGKILL			9
#define LINUX_SIGUSR1			10
#define LINUX_SIGSEGV			11
#define LINUX_SIGUSR2			12
#define LINUX_SIGPIPE			13
#define LINUX_SIGALRM			14
#define LINUX_SIGTERM			15
#define LINUX_SIGSTKFLT			16
#define LINUX_SIGCHLD			17
#define LINUX_SIGCONT			18
#define LINUX_SIGSTOP			19
#define LINUX_SIGTSTP			20
#define LINUX_SIGTTIN			21
#define LINUX_SIGTTOU			22
#define LINUX_SIGURG			23
#define LINUX_SIGXCPU			24
#define LINUX_SIGXFSZ			25
#define LINUX_SIGVTALRM			26
#define LINUX_SIGPROF			27
#define LINUX_SIGWINCH			28
#define LINUX_SIGIO				29
#define LINUX_SIGPOLL			LINUX_SIGIO
#define LINUX_SIGPWR			30
#define LINUX_SIGSYS			31
#define	LINUX_SIGUNUSED			31

#define LINUX_SIGRTMIN			32
#define LINUX_SIGRTMAX			LINUX__NSIG

#define LINUX_SA_NOCLDSTOP		0x00000001
#define LINUX_SA_NOCLDWAIT		0x00000002
#define LINUX_SA_SIGINFO		0x00000004
#define LINUX_SA_RESTORER		0x04000000
#define LINUX_SA_ONSTACK		0x08000000
#define LINUX_SA_RESTART		0x10000000
#define LINUX_SA_NODEFER		0x40000000
#define LINUX_SA_RESETHAND		0x80000000

#define LINUX_SA_NOMASK			LINUX_SA_NODEFER
#define LINUX_SA_ONESHOT		LINUX_SA_RESETHAND

typedef uint64_t				linux_sigset_t;
typedef uint32_t				linux_old_sigset_t;

typedef struct {

		linux__sighandler_t		sa_handler;
		linux_ulong_t			sa_flags;
		linux__sigrestore_t		sa_restorer;
		linux_sigset_t			sa_mask;

} linux_sigaction;

typedef struct {

		linux__sighandler_t		sa_handler;
		linux_old_sigset_t		sa_mask;
		linux_ulong_t			sa_flags;
		linux__sigrestore_t		sa_restorer;

} linux_old_sigaction;

typedef struct {

		void*					ss_sp;
		int						ss_flags;
		linux_size_t			ss_size;

} linux_stack_t;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_sigset_t		sigset_t;
	typedef linux_old_sigset_t	old_sigset_t;

	typedef linux_sigaction		sigaction;
	typedef linux_old_sigaction	old_sigaction;
	typedef linux_stack_t		stack_t;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------
// include/uapi/linux/signal.h
//-----------------------------------------------------------------------------

#define LINUX_SS_ONSTACK		1
#define LINUX_SS_DISABLE		2

//-----------------------------------------------------------------------------
// include/linux/signal.h
//-----------------------------------------------------------------------------

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	// Converts a signal identifier into a signal mask bit
	inline uapi::sigset_t sigmask(int signal)
	{
		return static_cast<uapi::sigset_t>(1) << (signal - 1);
	}

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_SIGNAL_H_