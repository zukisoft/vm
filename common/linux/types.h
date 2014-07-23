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

#ifndef __LINUX_TYPES_H_
#define __LINUX_TYPES_H_
#pragma once

//-----------------------------------------------------------------------------
// Standard Integer Types
//-----------------------------------------------------------------------------

#ifndef __midl
#include <stdint.h>
#else
typedef signed char					int8_t;
typedef unsigned char				uint8_t;
typedef short						int16_t;
typedef unsigned short				uint16_t;
typedef __int32						int32_t;
typedef unsigned __int32			uint32_t;
typedef __int64						int64_t;
typedef unsigned __int64			uint64_t;
#endif	// __midl

//-----------------------------------------------------------------------------
// include/uapi/linux/posix_types.h
//-----------------------------------------------------------------------------

typedef int							__kernel_key_t;
typedef int							__kernel_mqd_t;

//-----------------------------------------------------------------------------
// include/uapi/asm-generic/posix_types.h
//-----------------------------------------------------------------------------

typedef __int3264					__kernel_long_t;
typedef unsigned __int3264			__kernel_ulong_t;
typedef __kernel_ulong_t			__kernel_ino_t;
typedef unsigned int				__kernel_mode_t;
typedef int							__kernel_pid_t;
typedef int							__kernel_ipc_pid_t;
typedef unsigned int				__kernel_uid_t;
typedef unsigned int				__kernel_gid_t;
typedef __kernel_long_t				__kernel_suseconds_t;
typedef int							__kernel_daddr_t;
typedef unsigned int				__kernel_uid32_t;
typedef unsigned int				__kernel_gid32_t;
typedef __kernel_uid_t				__kernel_old_uid_t;
typedef __kernel_gid_t				__kernel_old_gid_t;
typedef unsigned int				__kernel_old_dev_t;
typedef unsigned __int3264			__kernel_size_t;
typedef __int3264					__kernel_ssize_t;
typedef __int3264					__kernel_ptrdiff_t;
typedef struct { int val[2]; }		__kernel_fsid_t;
typedef __kernel_long_t				__kernel_off_t;
typedef long long					__kernel_loff_t;
typedef __kernel_long_t				__kernel_time_t;
typedef __kernel_long_t				__kernel_clock_t;
typedef int							__kernel_timer_t;
typedef int							__kernel_clockid_t;
typedef char*						__kernel_caddr_t;
typedef unsigned short				__kernel_uid16_t;
typedef unsigned short				__kernel_gid16_t;

//-----------------------------------------------------------------------------
// include/linux/types.h
//-----------------------------------------------------------------------------

typedef uint32_t					__kernel_dev_t;

typedef __kernel_dev_t				linux_dev_t;
typedef __kernel_ino_t				linux_ino_t;
typedef __kernel_mode_t				linux_mode_t;
typedef unsigned short				linux_umode_t;
typedef uint32_t					linux_nlink_t;
typedef __kernel_off_t				linux_off_t;
typedef __kernel_pid_t				linux_pid_t;
typedef __kernel_daddr_t			linux_daddr_t;
typedef __kernel_key_t				linux_key_t;
typedef __kernel_suseconds_t		linux_suseconds_t;
typedef __kernel_timer_t			linux_timer_t;
typedef __kernel_clockid_t			linux_clockid_t;
typedef __kernel_mqd_t				linux_mqd_t;
typedef __kernel_uid32_t			linux_uid_t;
typedef __kernel_gid32_t			linux_gid_t;
typedef __kernel_uid16_t			linux_uid16_t;
typedef __kernel_gid16_t			linux_gid16_t;
typedef __kernel_loff_t				linux_loff_t;
typedef __kernel_size_t				linux_size_t;
typedef __kernel_ssize_t			linux_ssize_t;
typedef __kernel_ptrdiff_t			linux_ptrdiff_t;
typedef __kernel_time_t				linux_time_t;
typedef __kernel_clock_t			linux_clock_t;
typedef __kernel_caddr_t			linux_caddr_t;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_dev_t				dev_t;
	typedef linux_ino_t				ino_t;
	typedef linux_mode_t			mode_t;
	typedef linux_umode_t			umode_t;
	typedef linux_nlink_t			nlink_t;
	typedef linux_off_t				off_t;
	typedef linux_pid_t				pid_t;
	typedef linux_daddr_t			daddr_t;
	typedef linux_key_t				key_t;
	typedef linux_suseconds_t		suseconds_t;
	typedef linux_timer_t			timer_t;
	typedef linux_clockid_t			clockid_t;
	typedef linux_mqd_t				mqd_t;
	typedef linux_uid_t				uid_t;
	typedef linux_gid_t				gid_t;
	typedef linux_uid16_t			uid16_t;
	typedef linux_gid16_t			gid16_t;
	typedef linux_loff_t			loff_t;
	typedef linux_size_t			size_t;
	typedef linux_ssize_t			ssize_t;
	typedef linux_ptrdiff_t			ptrdiff_t;
	typedef linux_time_t			time_t;
	typedef linux_clock_t			clock_t;
	typedef linux_caddr_t			caddt_t;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_TYPES_H_