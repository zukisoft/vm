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

#ifndef __LINUX_SIGNAL_H_
#define __LINUX_SIGNAL_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/signal.h
//-----------------------------------------------------------------------------

#define LINUX__NSIG			64

#define LINUX_SIGHUP		1
#define LINUX_SIGINT		2
#define LINUX_SIGQUIT		3
#define LINUX_SIGILL		4
#define LINUX_SIGTRAP		5
#define LINUX_SIGABRT		6
#define LINUX_SIGIOT		6
#define LINUX_SIGBUS		7
#define LINUX_SIGFPE		8
#define LINUX_SIGKILL		9
#define LINUX_SIGUSR1		10
#define LINUX_SIGSEGV		11
#define LINUX_SIGUSR2		12
#define LINUX_SIGPIPE		13
#define LINUX_SIGALRM		14
#define LINUX_SIGTERM		15
#define LINUX_SIGSTKFLT		16
#define LINUX_SIGCHLD		17
#define LINUX_SIGCONT		18
#define LINUX_SIGSTOP		19
#define LINUX_SIGTSTP		20
#define LINUX_SIGTTIN		21
#define LINUX_SIGTTOU		22
#define LINUX_SIGURG		23
#define LINUX_SIGXCPU		24
#define LINUX_SIGXFSZ		25
#define LINUX_SIGVTALRM		26
#define LINUX_SIGPROF		27
#define LINUX_SIGWINCH		28
#define LINUX_SIGIO			29
#define LINUX_SIGPOLL		LINUX_SIGIO
#define LINUX_SIGPWR		30
#define LINUX_SIGSYS		31
#define	LINUX_SIGUNUSED		31

#define LINUX_SIGRTMIN		32
#define LINUX_SIGRTMAX		LINUX__NSIG

typedef uint64_t			linux_sigset_t;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_sigset_t		sigset_t;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------
// include/linux/signal.h
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

#endif		// __LINUX_SIGNAL_H_