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

#pragma warning(push, 4)				// Enable maximum compiler warnings

// void* mmap2(void *addr, size_t length, int prot, int flags, int fd, off_t pgoffset);
//
// EBX	- void*		addr
// ECX	- size_t	length
// EDX	- int		prot
// ESI	- int		flags
// EDI	- int		fd
// EBP	- off_t		pgoffset
//
int sys192_mmap2(PCONTEXT context)
{
	uint32_t flags = static_cast<uint32_t>(context->Esi);

	// Offset is specified as 4096-byte pages to allow for offsets greater than 2GiB on 32-bit systems
	ULARGE_INTEGER offset;
	offset.QuadPart = (context->Ebp * 4096);

	// Attempt to create the file mapping using the arguments provided by the caller
	HANDLE mapping = CreateFileMapping((flags & MAP_ANONYMOUS) ? INVALID_HANDLE_VALUE : reinterpret_cast<HANDLE>(context->Edi),
		NULL, ProtToPageFlags(context->Edx), 0, context->Ecx, NULL);
	if(mapping) {

		// Convert the prot and flags into FILE_MAP_XXXX flags for MapViewOfFile()
		uint32_t filemapflags = ProtToFileMapFlags(context->Edx);
		if(flags & MAP_PRIVATE) filemapflags |= FILE_MAP_COPY;

		// Attempt to map the file into memory using the constructed flags and offset values
		void* address = MapViewOfFileEx(mapping, filemapflags, offset.HighPart, offset.LowPart, context->Ecx, 
			reinterpret_cast<void*>(context->Ebx));

		// Always close the file mapping handle, it will remain active until the view is unmapped
		CloseHandle(mapping);

		return (address) ? reinterpret_cast<int>(address) : -1;
	}

	return -1;

	// MAP_FIXED - interpret addr as absolute, otherwise it's a hint!
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
