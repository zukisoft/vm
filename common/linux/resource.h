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

#ifndef __LINUX_RESOURCE_H_
#define __LINUX_RESOURCE_H_
#pragma once

#include "types.h"
#include "time.h"

//-----------------------------------------------------------------------------
// include/uapi/linux/resource.h
//-----------------------------------------------------------------------------

#define	LINUX_RUSAGE_SELF			0
#define	LINUX_RUSAGE_CHILDREN		(-1)
#define	LINUX_RUSAGE_BOTH			(-2)		/* sys_wait4() uses this */
#define	LINUX_RUSAGE_THREAD			1			/* only the calling thread */

// TODO: verify structure size
typedef struct {

	linux_timeval32			ru_utime;			/* user time used */
	linux_timeval32			ru_systime;			/* system time used */
	__int32					ru_maxrss;			/* maximum resident set size */
	__int32					ru_ixrss;			/* integral shared memory size */
	__int32					ru_idrss;			/* integral unshared data size */
	__int32					ru_isrss;			/* integral unshared stack size */
	__int32					ru_minflt;			/* page reclaims */
	__int32					ru_majflt;			/* page faults */
	__int32					ru_nswap;			/* swaps */
	__int32					ru_inblock;			/* block input operations */
	__int32					ru_oublock;			/* block output operations */
	__int32					ru_msgsnd;			/* messages sent */
	__int32					ru_msgrcv;			/* messages received */
	__int32					ru_nsignals;		/* signals received */
	__int32					ru_nvcsw;			/* voluntary context switches */
	__int32					ru_nivcsw;			/* involuntary " */

} linux_rusage32;

// TODO: verify structure size
typedef struct {

	linux_timeval64			ru_utime;			/* user time used */
	linux_timeval64			ru_systime;			/* system time used */
	__int64					ru_maxrss;			/* maximum resident set size */
	__int64					ru_ixrss;			/* integral shared memory size */
	__int64					ru_idrss;			/* integral unshared data size */
	__int64					ru_isrss;			/* integral unshared stack size */
	__int64					ru_minflt;			/* page reclaims */
	__int64					ru_majflt;			/* page faults */
	__int64					ru_nswap;			/* swaps */
	__int64					ru_inblock;			/* block input operations */
	__int64					ru_oublock;			/* block output operations */
	__int64					ru_msgsnd;			/* messages sent */
	__int64					ru_msgrcv;			/* messages received */
	__int64					ru_nsignals;		/* signals received */
	__int64					ru_nvcsw;			/* voluntary context switches */
	__int64					ru_nivcsw;			/* involuntary " */

} linux_rusage64;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	// todo
	typedef linux_rusage32			rusage;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_TIME_H_