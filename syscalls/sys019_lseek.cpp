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

// off_t lseek(int fd, off_t offset, int whence)
//
// EBX	- int			fd
// ECX	- off_t			offset
// EDX	- int			whence
// ESI
// EDI
// EBP
//
int sys019_lseek(PCONTEXT context)
{
	// Assumptions
	static_assert(LINUX_SEEK_SET == FILE_BEGIN, "Assumption (LINUX_SEEK_SET==FILE_BEGIN) is no longer true");
	static_assert(LINUX_SEEK_CUR == FILE_CURRENT, "Assumption (LINUX_SEEK_CUR==FILE_CURRENT) is no longer true");
	static_assert(LINUX_SEEK_END == FILE_END, "Assumption (LINUX_SEEK_END==FILE_END) is no longer true");

	_ASSERTE(context->Eax == 19);				// Verify system call number

	// Look up the specified file descriptor in the process descriptor table
	FileDescriptor fd = FileDescriptorTable::Get(static_cast<int32_t>(context->Ebx));
	if(fd == FileDescriptor::Null) return -LINUX_EBADF;

	// Sparse files aren't supported yet
	if(static_cast<uint32_t>(context->Edx) > SEEK_END) {

		_RPTF0(_CRT_ASSERT, "lseek: Sparse files have not been implemented yet");
		return -LINUX_EINVAL;
	}
	
	// Convert the distance into a 64-bit value, use QuadPart to ensure that
	// a negative value is properly sign-extended
	LARGE_INTEGER distance;
	distance.QuadPart = static_cast<off_t>(context->Ecx);

	// Change the file pointer
	LARGE_INTEGER pointer;
	if(!SetFilePointerEx(fd.OsHandle, distance, &pointer, context->Edx)) return -LINUX_EINVAL;

	// lseek should return EOVERFLOW if the new pointer would overflow
	return ((pointer.QuadPart > INT32_MAX) || (pointer.QuadPart < INT32_MIN)) ? -LINUX_EOVERFLOW : 
		static_cast<int>(pointer.QuadPart);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
