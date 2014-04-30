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

#include "stdafx.h"

#pragma warning(push, 4)				

// mmap2()
extern int sys192_mmap2(PCONTEXT);

// void* mmap(void *addr, size_t length, int prot, int flags, int fd, off_t offset)
//
// EBX	- void*		addr
// ECX	- size_t	length
// EDX	- int		prot
// ESI	- int		flags
// EDI	- int		fd
// EBP	- off_t		offset
//
int sys090_mmap(PCONTEXT context)
{
	_ASSERTE(context->Eax == 90);			// Verify system call number

	context->Eax = 192;						// Change to mmap2 syscall
	context->Ebp >>= 12;					// Convert to 4096-byte pages

	// Invoke mmap2() with the modified context registers
	return sys192_mmap2(context);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
