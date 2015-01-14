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

#ifndef __LINUX_TIME_H_
#define __LINUX_TIME_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// include/uapi/linux/time.h
//-----------------------------------------------------------------------------

typedef struct {

	__int32					tv_sec;				/* seconds */
	__int32					tv_nsec;			/* nanoseconds */

} linux_timespec32;

typedef struct {

	__int64					tv_sec;				/* seconds */
	__int64					tv_nsec;			/* nanoseconds */

} linux_timespec64;

typedef struct {

	__int32					tv_sec;				/* seconds */
	__int32					tv_usec;			/* microseconds */

} linux_timeval32;

typedef struct {

	__int64					tv_sec;				/* seconds */
	__int64					tv_usec;			/* microseconds */

} linux_timeval64;

typedef struct {

	int						tz_minuteswest;		/* minutes west of Greenwich */
	int						tz_dsttime;			/* type of dst correction */

} linux_timezone;

#define	LINUX_ITIMER_REAL					0
#define	LINUX_ITIMER_VIRTUAL				1
#define	LINUX_ITIMER_PROF					2

typedef struct {

	linux_timespec32		it_interval;		/* timer period */
	linux_timespec32		it_value;			/* timer expiration */

} linux_itimerspec32;

typedef struct {

	linux_timespec64		it_interval;		/* timer period */
	linux_timespec64		it_value;			/* timer expiration */

} linux_itimerspec64;

typedef struct {

	linux_timeval32			it_interval;		/* timer interval */
	linux_timeval32			it_value;			/* current value */

} linux_itimerval32;

typedef struct {

	linux_timeval64			it_interval;		/* timer interval */
	linux_timeval64			it_value;			/* current value */

} linux_itimerval64;

#define LINUX_CLOCK_REALTIME				0
#define LINUX_CLOCK_MONOTONIC				1
#define LINUX_CLOCK_PROCESS_CPUTIME_ID		2
#define LINUX_CLOCK_THREAD_CPUTIME_ID		3
#define LINUX_CLOCK_MONOTONIC_RAW			4
#define LINUX_CLOCK_REALTIME_COARSE			5
#define LINUX_CLOCK_MONOTONIC_COARSE		6
#define LINUX_CLOCK_BOOTTIME				7
#define LINUX_CLOCK_REALTIME_ALARM			8
#define LINUX_CLOCK_BOOTTIME_ALARM			9
#define LINUX_CLOCK_SGI_CYCLE				10	/* Hardware specific */
#define LINUX_CLOCK_TAI						11

#define LINUX_MAX_CLOCKS					16
#define LINUX_CLOCKS_MASK					(LINUX_CLOCK_REALTIME | LINUX_CLOCK_MONOTONIC)
#define LINUX_CLOCKS_MONO					LINUX_CLOCK_MONOTONIC

#define LINUX_TIMER_ABSTIME					0x01

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_timespec64		timespec;
	typedef linux_timeval64			timeval;
	typedef linux_timezone			timezone;
	typedef linux_itimerspec64		itimerspec;
	typedef linux_itimerval64		itimerval;

	// Converts Windows FILEFILE to Linux timespec
	//
	inline timespec FILETIMEToTimeSpec(const FILETIME& ft)
	{
		const __int64 OFFSET = 116444736000000000;
		__int64 filetime = *reinterpret_cast<const __int64*>(&ft);
		__int64 seconds = (filetime - OFFSET) / 10000000;
		__int64 nanoseconds = ((filetime - OFFSET) * 100) % 1000000000;

		return { static_cast<__kernel_time_t>(seconds), static_cast<__kernel_long_t>(nanoseconds) };
	}

	// Converts Windows FILEFILE to Linux timespec
	//
	inline void FILETIMEToTimeSpec(const FILETIME& ft, uint64_t* tv_sec, uint64_t* tv_nsec)
	{
		const __int64 OFFSET = 116444736000000000;
		__int64 filetime = *reinterpret_cast<const __int64*>(&ft);
		*tv_sec = (filetime - OFFSET) / 10000000;
		*tv_nsec = ((filetime - OFFSET) * 100) % 1000000000;
	}

	// Converts Linux timespec to Windows FILETIME
	//
	inline FILETIME TimeSpecToFILETIME(const timespec& ts)
	{
		const __int64 OFFSET = 116444736000000000;
		__int64 filetime = (static_cast<__int64>(ts.tv_sec) * 10000000) + (static_cast<__int64>(ts.tv_nsec) / 100) + OFFSET;

		return *reinterpret_cast<FILETIME*>(&filetime);
	}

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_TIME_H_