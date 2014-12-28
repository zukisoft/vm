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

#ifndef __LINUX_UTSNAME_H_
#define __LINUX_UTSNAME_H_
#pragma once

#include "types.h"

//-----------------------------------------------------------------------------
// include/uapi/linux/utsname.h
//-----------------------------------------------------------------------------

#define LINUX__OLD_UTS_LEN				8
#define LINUX__NEW_UTS_LEN				64

typedef struct {

	linux_char_t sysname[LINUX__OLD_UTS_LEN + 1];
	linux_char_t nodename[LINUX__OLD_UTS_LEN + 1];
	linux_char_t release[LINUX__OLD_UTS_LEN + 1];
	linux_char_t version[LINUX__OLD_UTS_LEN + 1];
	linux_char_t machine[LINUX__OLD_UTS_LEN + 1];

} linux_oldold_utsname;

typedef struct {

	linux_char_t sysname[LINUX__NEW_UTS_LEN + 1];
	linux_char_t nodename[LINUX__NEW_UTS_LEN + 1];
	linux_char_t release[LINUX__NEW_UTS_LEN + 1];
	linux_char_t version[LINUX__NEW_UTS_LEN + 1];
	linux_char_t machine[LINUX__NEW_UTS_LEN + 1];

} linux_old_utsname;

typedef struct {

	linux_char_t sysname[LINUX__NEW_UTS_LEN + 1];
	linux_char_t nodename[LINUX__NEW_UTS_LEN + 1];
	linux_char_t release[LINUX__NEW_UTS_LEN + 1];
	linux_char_t version[LINUX__NEW_UTS_LEN + 1];
	linux_char_t machine[LINUX__NEW_UTS_LEN + 1];
	linux_char_t domainname[LINUX__NEW_UTS_LEN + 1];

} linux_new_utsname;

#if !defined(__midl) && defined(__cplusplus)
namespace uapi {

	typedef linux_oldold_utsname	oldold_utsname;
	typedef linux_old_utsname		old_utsname;
	typedef linux_new_utsname		new_utsname;

}	// namespace uapi
#endif	// !defined(__midl) && defined(__cplusplus)

//-----------------------------------------------------------------------------

#endif		// __LINUX_UTSNAME_H_