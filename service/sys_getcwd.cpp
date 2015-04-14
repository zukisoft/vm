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
#include "FileSystem.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_getcwd
//
// Gets the current working directory as an absolute path
//
// Arguments:
//
//	context		- System call context object
//	buf			- Output buffer to receive the current working directory
//	size		- Length of the output buffer in bytes

uapi::long_t sys_getcwd(const Context* context, uapi::char_t* buf, size_t size)
{
	if(buf == nullptr) return -LINUX_EFAULT;

	// Ask the virtual machine instance to resolve the absolute path to the working directory
	FileSystem::GetAbsolutePath(context->Process->RootDirectory, context->Process->WorkingDirectory, buf, size);

	return 0;
}

// sys32_readlink
//
sys32_long_t sys32_getcwd(sys32_context_t context, sys32_char_t* buf, sys32_size_t size)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_getcwd, context, buf, size));
}

#ifdef _M_X64
// sys64_getcwd
//
sys64_long_t sys64_getcwd(sys64_context_t context, sys64_char_t* buf, sys64_sizeis_t size)
{
	return SystemCall::Invoke(sys_getcwd, context, buf, static_cast<size_t>(size));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
