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

#ifndef __LINUX_SIGINFO_H_
#define __LINUX_SIGINFO_H_
#pragma once

#include "types.h"

#pragma warning(disable:4201)		// nameless struct / union

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/siginfo.h
//-----------------------------------------------------------------------------

//
// TODO: THESE STRUCTURES WILL NEED TO BE SIZE-CHECKED AGAINST GLIBC/BIONIC
//

// todo: void* different sizes on x86/x64
typedef union {

	int32_t		sival_int;
	void*		sival_ptr;

} linux_sigval_t;

#define LINUX_SI_PREAMBLE_SIZE		(3 * sizeof(int32_t))
#define LINUX_SI_MAX_SIZE			128
#define LINUX_SI_PAD_SIZE			((LINUX_SI_MAX_SIZE - LINUX_SI_PREAMBLE_SIZE) / sizeof(int32_t))

#ifndef __ARCH_SI_UID_T
#define __ARCH_SI_UID_T	__kernel_uid32_t
#endif

/*
 * The default "si_band" type is "long", as specified by POSIX.
 * However, some architectures want to override this to "int"
 * for historical compatibility reasons, so we allow that.
 */
#ifndef __ARCH_SI_BAND_T
#define __ARCH_SI_BAND_T long
#endif

#ifndef __ARCH_SI_CLOCK_T
#define __ARCH_SI_CLOCK_T __kernel_clock_t
#endif

typedef struct {

	int32_t			si_signo;
	int32_t			si_errno;
	int32_t			si_code;

	union {

		int32_t		_pad[LINUX_SI_PAD_SIZE];

		/* kill() */
		struct {

			__kernel_pid_t		_pid;			/* sender's pid */
			__kernel_uid32_t	_uid;			/* sender's uid */

		} _kill;

		/* POSIX.1b timers */
		struct {

			__kernel_timer_t	_tid;			/* timer id */
			int32_t				_overrun;		/* overrun count */
			char				_pad[8];		/* sizeof(__kernel_uid32_t) - sizeof(int32_t) */
			linux_sigval_t		_sigval;		/* same as below */
			int32_t				_sys_private;	/* not to be passed to user */

		} _timer;

		/* POSIX.1b signals */
		struct {
			
			__kernel_pid_t		_pid;			/* sender's pid */
			__kernel_uid32_t	_uid;			/* sender's uid */
			linux_sigval_t		_sigval;

		} _rt;

		/* SIGCHLD */
		struct {

			__kernel_pid_t		_pid;			/* which child */
			__kernel_uid32_t	_uid;			/* sender's uid */
			int32_t				_status;		/* exit code */
			__kernel_clock_t	_utime;
			__kernel_clock_t	_stime;

		} _sigchld;

		/* SIGILL, SIGFPE, SIGSEGV, SIGBUS */
		struct {

			void*				_addr;			/* faulting insn/memory ref. */					// <--- void* here, be careful
			int16_t				_addr_lsb;		/* LSB of the reported address */

		} _sigfault;

		/* SIGPOLL */
		struct {

			__ARCH_SI_BAND_T	_band;			/* POLL_IN, POLL_OUT, POLL_MSG */		// <---- LONG here, be careful
			int32_t				_fd;

		} _sigpoll;

		/* SIGSYS */
		struct {
			
			void*				_call_addr;		/* calling user insn */						// <--- void* here
			int32_t				_syscall;		/* triggering system call number */
			uint32_t			_arch;			/* AUDIT_ARCH_* of syscall */

		} _sigsys;

	} _sifields;

} linux_siginfo32;

#define linux_si_pid			_sifields._kill._pid
#define linux_si_uid			_sifields._kill._uid
#define linux_si_tid			_sifields._timer._tid
#define linux_si_overrun		_sifields._timer._overrun
#define linux_si_sys_private	_sifields._timer._sys_private
#define linux_si_status			_sifields._sigchld._status
#define linux_si_utime			_sifields._sigchld._utime
#define linux_si_stime			_sifields._sigchld._stime
#define linux_si_value			_sifields._rt._sigval
#define linux_si_int			_sifields._rt._sigval.sival_int
#define linux_si_ptr			_sifields._rt._sigval.sival_ptr
#define linux_si_addr			_sifields._sigfault._addr
#define linux_si_addr_lsb		_sifields._sigfault._addr_lsb
#define linux_si_band			_sifields._sigpoll._band
#define linux_si_fd				_sifields._sigpoll._fd

#define LINUX_CLD_EXITED		1				/* child has exited */
#define LINUX_CLD_KILLED		2				/* child was killed */
#define LINUX_CLD_DUMPED		3				/* child terminated abnormally */
#define LINUX_CLD_TRAPPED		4				/* traced child has trapped */
#define LINUX_CLD_STOPPED		5				/* child has stopped */
#define LINUX_CLD_CONTINUED		6				/* stopped child has continued */
#define LINUX_NSIGCHLD			6

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	// todo
	typedef linux_siginfo32		siginfo;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_SIGINFO_H_