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

#ifndef __LINUX_KERN_LEVELS_H_
#define __LINUX_KERN_LEVELS_H_
#pragma once

//-----------------------------------------------------------------------------
// include/linux/kern_levels.h
//-----------------------------------------------------------------------------

#define LINUX_KERN_SOH			"\001"			/* ASCII Start Of Header */
#define LINUX_KERN_SOH_ASCII	'\001'

#define LINUX_KERN_EMERG	LINUX_KERN_SOH "0"	/* system is unusable */
#define LINUX_KERN_ALERT	LINUX_KERN_SOH "1"	/* action must be taken immediately */
#define LINUX_KERN_CRIT		LINUX_KERN_SOH "2"	/* critical conditions */
#define LINUX_KERN_ERR		LINUX_KERN_SOH "3"	/* error conditions */
#define LINUX_KERN_WARNING	LINUX_KERN_SOH "4"	/* warning conditions */
#define LINUX_KERN_NOTICE	LINUX_KERN_SOH "5"	/* normal but significant condition */
#define LINUX_KERN_INFO		LINUX_KERN_SOH "6"	/* informational */
#define LINUX_KERN_DEBUG	LINUX_KERN_SOH "7"	/* debug-level messages */

#define LINUX_KERN_DEFAULT	LINUX_KERN_SOH "d"	/* the default kernel loglevel */

/*
 * Annotation for a "continued" line of log printout (only done after a
 * line that had no enclosing \n). Only to be used by core/arch code
 * during early bootup (a continued line is not SMP-safe otherwise).
 */
#define LINUX_KERN_CONT			""

/* integer equivalents of KERN_<LEVEL> */
#define LINUX_LOGLEVEL_SCHED		-2			/* Deferred messages from sched code are set to this special level */
#define LINUX_LOGLEVEL_DEFAULT		-1			/* default (or last) loglevel */
#define LINUX_LOGLEVEL_EMERG		0			/* system is unusable */
#define LINUX_LOGLEVEL_ALERT		1			/* action must be taken immediately */
#define LINUX_LOGLEVEL_CRIT			2			/* critical conditions */
#define LINUX_LOGLEVEL_ERR			3			/* error conditions */
#define LINUX_LOGLEVEL_WARNING		4			/* warning conditions */
#define LINUX_LOGLEVEL_NOTICE		5			/* normal but significant condition */
#define LINUX_LOGLEVEL_INFO			6			/* informational */
#define LINUX_LOGLEVEL_DEBUG		7			/* debug-level messages */

//-----------------------------------------------------------------------------

#endif		// __LINUX_KERN_LEVELS_H_