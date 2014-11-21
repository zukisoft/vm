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
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_mount
//
// Mounts a file system
//
// Arguments:
//
//	context		- SystemCall context object
//	source		- Source device/object to be mounted
//	target		- Target directory on which to mount the filesystem
//	filesystem	- Name of the filesystem to use for the device/object
//	flags		- Mount options and flags
//	data		- Address of additional mounting options (NOTE: CLIENT PROCESS ADDRESS) 

__int3264 sys_mount(const SystemCall::Context* context, const uapi::char_t* source, const uapi::char_t* target, const uapi::char_t* filesystem, uint32_t flags, const void* data)
{
	_ASSERTE(context);

	try {
		
		SystemCall::Impersonation impersonation;

		// Custom mounting data that needs to be copied from the client process
		if(data != nullptr) {

			// The way the data argument appears to work in the Linux kernel without a known buffer size is that
			// it will try to copy out up to PAGE_SIZE bytes of data and just stop if it encounters an issue
			HeapBuffer<uint8_t> datapage(MemoryRegion::PageSize);
			size_t datalen = context->Process->ReadMemory(data, &datapage, datapage.Size);

			// Invoke the VirtualMachine with the custom mounting data read from the process
			context->VirtualMachine->MountFileSystem(source, target, filesystem, flags, &datapage, datalen);
		}

		// No custom mounting data, just invoke the VirtualMachine without it
		else context->VirtualMachine->MountFileSystem(source, target, filesystem, flags, nullptr, 0);
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return 0;
}

// sys32_mount
//
sys32_long_t sys32_mount(sys32_context_t context, const sys32_char_t* source, const sys32_char_t* target, const sys32_char_t* filesystem, sys32_ulong_t flags, sys32_addr_t data)
{
	return static_cast<sys32_long_t>(sys_mount(reinterpret_cast<SystemCall::Context*>(context), source, target, filesystem, flags, reinterpret_cast<void*>(data)));
}

#ifdef _M_X64
// sys64_mount
//
sys64_long_t sys64_mount(sys64_context_t context, const sys64_char_t* source, const sys64_char_t* target, const sys64_char_t* filesystem, sys64_ulong_t flags, sys64_addr_t data)
{
	return sys_mount(reinterpret_cast<SystemCall::Context*>(context), source, target, filesystem, static_cast<uint32_t>(flags), reinterpret_cast<void*>(data));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
