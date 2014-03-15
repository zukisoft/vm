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

// EAX = 243
// EBX = struct user_desc*
void sys_set_thread_area(PCONTEXT context)
{
	struct user_desc* desc = reinterpret_cast<struct user_desc*>(context->Ebx);
	
	//
	// TODO: Need to verify user_desc is compatible here, it more or less describes an LDT
	// (see fill_ldt in arch/x86/include/asm/desc.h)
	//

	desc->entry_number = static_cast<uint32_t>(TlsAlloc());
	if(desc->entry_number != TLS_OUT_OF_INDEXES) {

		//void* mem = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, desc->limit);
		//if(!mem) { /* deal with this */ }
		TlsSetValue(desc->entry_number, reinterpret_cast<void*>(desc->base_addr));
	}
	
	// desc->base_address is seemingly important here, perhaps it needs to be copied

	// TODO: NEED ERRNO CODES NOT -1 !!!!!!!!

	context->Eax = 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)