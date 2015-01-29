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
#include "ContextHandle.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_sethostname
//
// Changes the host name reported by the virtual machine
//
// Arguments:
//
//	context		- SystemCall context object
//	name		- New host name string to be assigned
//	len			- Length of the name string, does not include null terminator

__int3264 sys_sethostname(const ContextHandle* context, uapi::char_t* name, size_t len)
{
	_ASSERTE(context);
	if(name == nullptr) return -LINUX_EFAULT;
	if(len == 0) return -LINUX_EINVAL;

	try { 
		
		context->VirtualMachine->SetProperty(VirtualMachine::Properties::HostName, name, len); 
	}
	
	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return 0;
}

// sys32_sethostname
//
sys32_long_t sys32_sethostname(sys32_context_t context, sys32_char_t* name, sys32_size_t len)
{
	return static_cast<sys32_long_t>(sys_sethostname(reinterpret_cast<ContextHandle*>(context), name, len));
}

#ifdef _M_X64
// sys64_sethostname
//
sys64_long_t sys64_sethostname(sys64_context_t context, sys64_char_t* name, sys64_sizeis_t len)
{
	return sys_sethostname(reinterpret_cast<ContextHandle*>(context), name, len);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
