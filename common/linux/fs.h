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

#ifndef __LINUX_FS_H_
#define __LINUX_FS_H_
#pragma once

//-----------------------------------------------------------------------------
// include/uapi/linux/fs.h
//-----------------------------------------------------------------------------

#define LINUX_SEEK_SET		0	/* seek relative to beginning of file */
#define LINUX_SEEK_CUR		1	/* seek relative to current file position */
#define LINUX_SEEK_END		2	/* seek relative to end of file */
#define LINUX_SEEK_DATA		3	/* seek to the next data */
#define LINUX_SEEK_HOLE		4	/* seek to the next hole */
#define LINUX_SEEK_MAX		LINUX_SEEK_HOLE

//-----------------------------------------------------------------------------

#endif		// __LINUX_FS_H_