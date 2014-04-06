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

struct user_desc {
	uint32_t entry_number;
	uint32_t base_addr;
	uint32_t limit;
	uint32_t seg_32bit:1;
	uint32_t contents:2;
	uint32_t read_exec_only:1;
	uint32_t limit_in_pages:1;
	uint32_t seg_not_present:1;
	uint32_t useable:1;
#ifdef _M_X64
	uint32_t lm:1;
#endif
};

// pid_t gettid(void);
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
	// Cast out and check the structure pointer for NULL
	struct user_desc* desc = reinterpret_cast<struct user_desc*>(context->Ebx);
	if(!desc) return -LINUX_EFAULT;

	// Windows doesn't allow us to set a specific TLS slot
	if(desc->entry_number != -1) return -LINUX_EINVAL;

	//
	// TODO: Need to verify user_desc is compatible here, it more or less describes an LDT
	// (see fill_ldt in arch/x86/include/asm/desc.h)
	//

	// Allocate a Thread Local Storage slot for the calling process
	uint32_t slot = TlsAlloc();
	if(slot == TLS_OUT_OF_INDEXES) return -LINUX_ESRCH;

	// Libc/Bionic will turn around and put the returned slot number into the GS
	// segment register.  Unfortunately, if the munged slot number happens to be
	// valid, that call will not raise an access violation and the vectored handler
	// won't be triggered.  For now I'm shifing it left 8 bits to try and guarantee
	// it won't bump into anything, but this results in a maximum slot number of 31
	if(slot > 31) { TlsFree(slot); return -LINUX_ESRCH; }

	desc->entry_number = (slot << 8);
	TlsSetValue(slot, reinterpret_cast<void*>(desc->base_addr));

	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
