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
#include "uapi.h"						// Include Linux UAPI declarations
#include "FileDescriptor.h"				// Include FileDescriptor declarations
#include "FileDescriptorTable.h"		// Include FileDescriptorTable decls

#pragma warning(push, 4)				// Enable maximum compiler warnings

// int __llseek(int fd, unsigned long offset_high, unsigned long offset_low, loff_t* result, int whence);
//
// EBX	- int			fd
// ECX	- unsigned long offset_high
// EDX	- unsigned long	offset_low
// ESI	- loff_t*		result
// EDI	- int			whence
// EBP
//
int sys140_llseek(PCONTEXT context)
{
	//// Look up the specified file descriptor in the process descriptor table
	//FileDescriptor fd = FileDescriptorTable::Get(static_cast<int32_t>(context->Ebx));
	//if(fd == FileDescriptor::Null) return -LINUX_EBADF;

	//// Convert the 32-bit offset components into a ULARGE_INTEGER
	//LARGE_INTEGER offset;
	//offset.HighPart = static_cast<int32_t>(context->Ecx);
	//offset.LowPart = static_cast<int32_t>(context->Edx);

	//// Check result for a bad pointer
	//if(context->Esi == 0) return -LINUX_EFAULT;

	//// TESTING (PHYSICAL FILES ASSUMED)
	//DWORD bytesread;
	//if(!ReadFile(fd.OsHandle, reinterpret_cast<void*>(context->Ecx), static_cast<DWORD>(context->Edx), &bytesread, nullptr)) {
	//	
	//	// Process error codes here
	//}

	//return static_cast<int>(bytesread);

	return -LINUX_ENOSYS;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
