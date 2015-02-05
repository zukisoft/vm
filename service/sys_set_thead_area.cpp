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
#include "SystemCall.h"
#include "SystemInformation.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// TODOTODO

uapi::long_t sys_set_thread_area(const Context* context, uapi::user_desc32* u_info)
{

	// STUB STUB STUB TODO TODO TODO

	u_info->entry_number = 0;
	context->Process->WriteMemory(context->Process->TESTLDT, u_info, sizeof(uapi::user_desc32));
	return 0;
}

// sys32_set_thread_area
//
sys32_long_t sys32_set_thread_area(sys32_context_t context, linux_user_desc32* u_info)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_set_thread_area, context, reinterpret_cast<uapi::user_desc32*>(u_info)));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
