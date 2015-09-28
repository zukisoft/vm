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

#include "SystemCallContext.h"
#include "_VMOld.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_newuname
//
// Gets information about the current virtual kernel
//
// Arguments:
//
//	context		- System call context object
//	buf			- Pointer to the output data structure

uapi::long_t sys_newuname(const Context* context, uapi::new_utsname* buf)
{
	return -LINUX_ENOSYS;

	//if(buf == nullptr) return -LINUX_EFAULT;

	//auto vm = context->_VmOld;

	//vm->GetProperty(_VmOld::Properties::OperatingSystemType,	buf->sysname,		LINUX__NEW_UTS_LEN + 1);
	//vm->GetProperty(_VmOld::Properties::HostName,				buf->nodename,		LINUX__NEW_UTS_LEN + 1);
	//vm->GetProperty(_VmOld::Properties::OperatingSystemRelease, buf->release,		LINUX__NEW_UTS_LEN + 1);
	//vm->GetProperty(_VmOld::Properties::OperatingSystemVersion, buf->version,		LINUX__NEW_UTS_LEN + 1);
	//vm->GetProperty(_VmOld::Properties::HardwareIdentifier,		buf->machine,		LINUX__NEW_UTS_LEN + 1);
	//vm->GetProperty(_VmOld::Properties::DomainName,				buf->domainname,	LINUX__NEW_UTS_LEN + 1);

	//return 0;
}

// sys32_newuname
//
sys32_long_t sys32_newuname(sys32_context_t context, uapi::new_utsname* buf)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_newuname, context, buf));
}

#ifdef _M_X64
// sys64_newuname
//
sys64_long_t sys64_newuname(sys64_context_t context, uapi::new_utsname* buf)
{
	return SystemCall::Invoke(sys_newuname, context, buf);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
