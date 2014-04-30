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
#include <linux/fs.h>
#include "FileDescriptor.h"
#include "FileDescriptorTable.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

// int __llseek(int fd, unsigned long offset_high, unsigned long offset_low, loff_t* result, int whence)
//
// EBX	- int			fd
// ECX	- unsigned long offset_high
// EDX	- unsigned long	offset_low
// ESI	- loff_t*		result
// EDI	- int			whence
// EBP
//
int sys140__llseek(PCONTEXT context)
{
	// Assumptions
	static_assert(LINUX_SEEK_SET == FILE_BEGIN, "Assumption (LINUX_SEEK_SET==FILE_BEGIN) is no longer true");
	static_assert(LINUX_SEEK_CUR == FILE_CURRENT, "Assumption (LINUX_SEEK_CUR==FILE_CURRENT) is no longer true");
	static_assert(LINUX_SEEK_END == FILE_END, "Assumption (LINUX_SEEK_END==FILE_END) is no longer true");

	_ASSERTE(context->Eax == 140);					// Verify system call number

	// Look up the specified file descriptor in the process descriptor table
	FileDescriptor fd = FileDescriptorTable::Get(static_cast<int32_t>(context->Ebx));
	if(fd == FileDescriptor::Null) return -LINUX_EBADF;

	// Check the output argument pointer
	if(context->Esi == 0) return -LINUX_EFAULT;

	// Sparse files aren't supported yet
	if(static_cast<uint32_t>(context->Edi) > SEEK_END) {

		_RPTF0(_CRT_ASSERT, "lseek: Sparse files have not been implemented yet");
		return -LINUX_EINVAL;
	}
	
	// Convert the distance into a 64-bit value, use QuadPart and bit shifting, that is how
	// the value would have been set on the usermode side
	LARGE_INTEGER distance;
	uint64_t offset = static_cast<uint64_t>(context->Ecx) << 32 | static_cast<uint32_t>(context->Edx);
	distance.QuadPart = static_cast<int64_t>(offset);

	// Change the file pointer
	LARGE_INTEGER pointer;
	if(!SetFilePointerEx(fd.OsHandle, distance, &pointer, context->Edi)) return -LINUX_EINVAL;

	// __llseek returns the new file pointer as a 64-bit signed integer
	*reinterpret_cast<uapi::loff_t*>(context->Esi) = pointer.QuadPart;

	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
