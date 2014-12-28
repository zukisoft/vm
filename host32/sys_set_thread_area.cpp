//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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
#include <linux/errno.h>
#include <linux/ldt.h>

#pragma warning(push, 4)

// t_ldt
//
// Thread-local LDT
extern __declspec(thread) sys32_ldt_t t_ldt;

// AllocateLDTEntry (emulator.cpp)
//
sys32_ldt_entry_t* AllocateLDTEntry(sys32_ldt_t* ldt, sys32_ldt_entry_t* entry);

//-----------------------------------------------------------------------------
// sys_set_thread_area
//
// Arguments:
//
//	u_info		- Thread-local storage descriptor

uapi::long_t sys_set_thread_area(uapi::user_desc* u_info)
{
	if(u_info == nullptr) return -LINUX_EFAULT;

	// Attempt to allocate/alter the entry at the specified position in the LDT
	sys32_ldt_entry_t* result = AllocateLDTEntry(&t_ldt, reinterpret_cast<sys32_ldt_entry_t*>(u_info));
	if(result == nullptr) return -LINUX_ESRCH;

	// Return the resultant entry number back through the user_desc structure upon success
	//
	// TODO: THIS VALUE IS HACKED LIKE IT WAS - THIS NEEDS TO BE CLEANED UP AND DOCUMENTED
	// IN ADDITION A NON -1 ENTRY_NUMBER IS NOT LIKELY TO WORK

	u_info->entry_number = ((result->entry_number + 1) << 8);
	return 0;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


