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
#include <linux/ldt.h>

#pragma warning(push, 4)				// Enable maximum compiler warnings

// int set_thread_area(struct user_desc* u_info); 
//
// EBX	- struct user_desc*
// ECX
// EDX
// ESI
// EDI
// EBP
//
int sys243_set_thread_area(PCONTEXT context)
{
	_ASSERTE(context->Eax == 243);				// Verify system call number

	// Cast out and check the structure pointer for NULL
	struct uapi::user_desc* desc = reinterpret_cast<struct uapi::user_desc*>(context->Ebx);
	if(!desc) return -LINUX_EFAULT;

	// Windows doesn't allow us to set a specific TLS slot
	if(desc->entry_number != -1) return -LINUX_EINVAL;

	// Allocate a Thread Local Storage slot for the calling process
	uint32_t slot = TlsAlloc();
	if(slot == TLS_OUT_OF_INDEXES) return -LINUX_ESRCH;

	// LIBC/Bionic will turn around and put the returned slot number into the GS
	// segment register.  Unfortunately, if the munged slot number happens to be
	// valid, that call will not raise an access violation and the vectored handler
	// won't be triggered.  For now I'm shifting it left 8 bits to try and guarantee
	// it won't bump into anything, but this results in a maximum slot number of 31
	if(slot > 31) { TlsFree(slot); return -LINUX_ESRCH; }

	desc->entry_number = (slot << 8);
	TlsSetValue(slot, reinterpret_cast<void*>(desc->base_addr));

	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
