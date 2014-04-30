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

#pragma warning(push, 4)				// Enable maximum compiler warnings

// ssize_t write(int fd, const void *buf, size_t count);
//
// EBX	- int			fd
// ECX	- const void*	buf
// EDX	- size_t		count
// ESI
// EDI
// EBP
//
int sys004_write(PCONTEXT context)
{
	_ASSERTE(context->Eax == 4);				// Verify system call number

	int fd = static_cast<int>(context->Ebx);
	const void* buf = reinterpret_cast<const void*>(context->Ecx);
	size_t count = static_cast<size_t>(context->Edx);

	/// TESTING (STDOUT / STDERR)
	if((fd == 1) || (fd == 2)) {

		std::string output(reinterpret_cast<const char*>(buf), count);
		OutputDebugStringA(output.c_str());
		return count;
	}

	return -LINUX_ENOSYS;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
