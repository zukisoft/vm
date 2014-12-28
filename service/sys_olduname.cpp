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

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_olduname
//
// Gets information about the current virtual kernel
//
// Arguments:
//
//	context		- SystemCall context object
//	buf			- Pointer to the output data structure

__int3264 sys_olduname(const SystemCall::Context* context, uapi::oldold_utsname* buf)
{
	_ASSERTE(context);
	if(buf == nullptr) return -LINUX_EFAULT;

	try {

		SystemCall::Impersonation impersonation;
		auto vm = context->VirtualMachine;

		vm->GetProperty(VirtualMachine::Properties::OperatingSystemType,	buf->sysname,	LINUX__OLD_UTS_LEN + 1);
		vm->GetProperty(VirtualMachine::Properties::HostName,				buf->nodename,	LINUX__OLD_UTS_LEN + 1);
		vm->GetProperty(VirtualMachine::Properties::OperatingSystemRelease, buf->release,	LINUX__OLD_UTS_LEN + 1);
		vm->GetProperty(VirtualMachine::Properties::OperatingSystemVersion, buf->version,	LINUX__OLD_UTS_LEN + 1);
		vm->GetProperty(VirtualMachine::Properties::HardwareIdentifier,		buf->machine,	LINUX__OLD_UTS_LEN + 1);
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return 0;
}

// sys32_olduname
//
sys32_long_t sys32_olduname(sys32_context_t context, uapi::oldold_utsname* buf)
{
	return static_cast<sys32_long_t>(sys_olduname(reinterpret_cast<SystemCall::Context*>(context), buf));
}

//---------------------------------------------------------------------------

#pragma warning(pop)
