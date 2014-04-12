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

#include "stdafx.h"						// Include project pre-compiled headers
#include "FileDescriptor.h"				// Include FileDescriptor declarations
#include "FileDescriptorTable.h"		// Include FileDescriptorTable decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

// int close(int fd);
//
// EBX	- int			fd
// ECX
// EDX
// ESI
// EDI
// EBP
//
int sys006_close(PCONTEXT context)
{
	int32_t fd = static_cast<int32_t>(context->Ebx);		// Cast out the fd

	// Check that the file descriptor is valid for this process
	if(FileDescriptorTable::Get(fd) == FileDescriptor::Null) return -LINUX_EBADF;

	// Release the file descriptor
	FileDescriptorTable::Free(fd);
	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
