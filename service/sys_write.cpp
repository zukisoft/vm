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
// sys_write
//
// Writes data to an open file system object
//
// Arguments:
//
//	context		- SystemCall context object
//	fd			- File descriptor
//	buf			- Input buffer containing the data to be written
//	count		- Number of bytes written to the file system object

__int3264 sys_write(const ContextHandle* context, int fd, const void* buf, size_t count)
{
	_ASSERTE(context);

	try { 
		
		return context->Process->GetHandle(fd)->Write(buf, count); 
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_write
//
sys32_long_t sys32_write(sys32_context_t context, sys32_int_t fd, const sys32_uchar_t* buf, sys32_size_t count)
{
	return static_cast<sys32_long_t>(sys_write(reinterpret_cast<ContextHandle*>(context), fd, buf, count));
}

#ifdef _M_X64
// sys64_write
//
sys64_long_t sys64_write(sys64_context_t context, sys64_int_t fd, const sys64_uchar_t* buf, sys64_sizeis_t count)
{
	return sys_write(reinterpret_cast<ContextHandle*>(context), fd, buf, count);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
