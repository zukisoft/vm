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
#include "FileDescriptor.h"
#include "FileDescriptorTable.h"

#pragma warning(push, 4)				

// ssize_t read(int fd, void *buf, size_t count);
//
// EBX	- int			fd
// ECX	- void*			buf
// EDX	- size_t		count
// ESI
// EDI
// EBP
//
int sys003_read(PCONTEXT context)
{
	_ASSERTE(context->Eax == 3);				// Verify system call number

	// Look up the specified file descriptor in the process descriptor table
	FileDescriptor fd = FileDescriptorTable::Get(static_cast<int32_t>(context->Ebx));
	if(fd == FileDescriptor::Null) return -LINUX_EBADF;

	// TESTING (PHYSICAL FILES ASSUMED)
	DWORD bytesread;
	if(!ReadFile(fd.OsHandle, reinterpret_cast<void*>(context->Ecx), static_cast<DWORD>(context->Edx), &bytesread, nullptr)) {
		
		// Process error codes here
	}

	return static_cast<int>(bytesread);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
