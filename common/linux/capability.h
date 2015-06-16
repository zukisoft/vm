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

#ifndef __LINUX_CAPABILITY_H_
#define __LINUX_CAPIBILITY_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/capability.h
//-----------------------------------------------------------------------------

#define LINUX_CAP_CHOWN					0
#define LINUX_CAP_DAC_OVERRIDE			1
#define LINUX_CAP_DAC_READ_SEARCH		2
#define LINUX_CAP_FOWNER				3
#define LINUX_CAP_FSETID				4
#define LINUX_CAP_KILL					5
#define LINUX_CAP_SETGID				6
#define LINUX_CAP_SETUID				7
#define LINUX_CAP_SETPCAP				8
#define LINUX_CAP_LINUX_IMMUTABLE		9
#define LINUX_CAP_NET_BIND_SERVICE		10
#define LINUX_CAP_NET_BROADCAST			11
#define LINUX_CAP_NET_ADMIN				12
#define LINUX_CAP_NET_RAW				13
#define LINUX_CAP_IPC_LOCK				14
#define LINUX_CAP_IPC_OWNER				15
#define LINUX_CAP_SYS_MODULE			16
#define LINUX_CAP_SYS_RAWIO				17
#define LINUX_CAP_SYS_CHROOT			18
#define LINUX_CAP_SYS_PTRACE			19
#define LINUX_CAP_SYS_PACCT				20
#define LINUX_CAP_SYS_ADMIN				21
#define LINUX_CAP_SYS_BOOT				22
#define LINUX_CAP_SYS_NICE				23
#define LINUX_CAP_SYS_RESOURCE			24
#define LINUX_CAP_SYS_TIME				25
#define LINUX_CAP_SYS_TTY_CONFIG		26
#define LINUX_CAP_MKNOD					27
#define LINUX_CAP_LEASE					28
#define LINUX_CAP_AUDIT_WRITE			29
#define LINUX_CAP_AUDIT_CONTROL			30
#define LINUX_CAP_SETFCAP				31
#define LINUX_CAP_MAC_OVERRIDE			32
#define LINUX_CAP_MAC_ADMIN				33
#define LINUX_CAP_SYSLOG				34
#define LINUX_CAP_WAKE_ALARM			35
#define LINUX_CAP_BLOCK_SUSPEND			36
#define LINUX_CAP_AUDIT_READ			37

//-----------------------------------------------------------------------------

#endif		// __LINUX_CAPABILITY_H_