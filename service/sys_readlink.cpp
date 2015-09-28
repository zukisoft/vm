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
#include "FileSystem.h"
#include "Process.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_readlink
//
// Reads the value of a symbolic link
//
// Arguments:
//
//	context		- System call context object
//	pathname	- Relative path to the symbolic link object
//	buf			- Output buffer
//	bufsiz		- Length of the output buffer, in bytes

uapi::long_t sys_readlink(const Context* context, const uapi::char_t* pathname, uapi::char_t* buf, size_t bufsiz)
{
	return -LINUX_ENOSYS;

	//if(buf == nullptr) return -LINUX_EFAULT;
	//return FileSystem::ReadSymbolicLink(context->Process->RootDirectory, context->Process->WorkingDirectory, pathname, buf, bufsiz);
}

// sys32_readlink
//
sys32_long_t sys32_readlink(sys32_context_t context, const sys32_char_t* pathname, sys32_char_t* buf, sys32_size_t bufsiz)
{
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_readlink, context, pathname, buf, bufsiz));
}

#ifdef _M_X64
// sys64_readlink
//
sys64_long_t sys64_readlink(sys64_context_t context, const sys64_char_t* pathname, sys64_char_t* buf, sys64_sizeis_t bufsiz)
{
	return SystemCall::Invoke(sys_readlink, context, pathname, buf, static_cast<size_t>(bufsiz));
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
