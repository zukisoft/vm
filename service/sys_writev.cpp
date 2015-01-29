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
#include "HeapBuffer.h"
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_writev
//
// Writes an array of blocks of data to an open file system object
//
// Arguments:
//
//	context		- SystemCall context object
//	fd			- Open file descriptor for the target object
//	iov			- Array of iovec structures for the operation
//	iovcnt		- Number of iovec structures provided via iov

uapi::long_t sys_writev(const ContextHandle* context, int fd, uapi::iovec* iov, int iovcnt)
{
	_ASSERTE(context);

	if(iov == nullptr) return -LINUX_EFAULT;
	if(iovcnt <= 0) return -LINUX_EINVAL;

	// Get the handle represented by the file descriptor
	auto handle = context->Process->GetHandle(fd);

	// Calcluate the maximum required intermediate data buffer for the operation
	size_t max = 0;
	for(int index = 0; index < iovcnt; index++) { if(iov[index].iov_len > max) max = iov[index].iov_len; }
	if(max == 0) throw LinuxException(LINUX_EINVAL);

	// Allocate the intermediate buffer
	HeapBuffer<uint8_t> buffer(max);

	// Repeatedly read the data from the child process address space and write it through the handle
	size_t written = 0;
	for(int index = 0; index < iovcnt; index++) {

		size_t read = context->Process->ReadMemory(iov[index].iov_base, buffer, iov[index].iov_len);
		if(read) written += handle->Write(buffer, read);
	}

	// Return the total number of bytes written; should be checking for an overflow here (EINVAL)
	return static_cast<uapi::long_t>(written);
}

#ifndef _M_X64
// sys32_writev (32-bit)
//
sys32_long_t sys32_writev(sys32_context_t context, sys32_int_t fd, sys32_iovec_t* iov, sys32_int_t iovcnt)
{
	static_assert(sizeof(uapi::iovec) == sizeof(sys32_iovec_t), "uapi::iovec is not equivalent to sys32_iovec_t");
	return SystemCall::Invoke(sys_writev, reinterpret_cast<ContextHandle*>(context), fd, reinterpret_cast<uapi::iovec*>(iov), iovcnt);
}
#else
// sys32_writev (64-bit)
//
sys32_long_t sys32_writev(sys32_context_t context, sys32_int_t fd, sys32_iovec_t* iov, sys32_int_t iovcnt)
{
	if(iov == nullptr) return -LINUX_EFAULT;
	if(iovcnt <= 0) return -LINUX_EINVAL;

	try {

		// uapi::iovec and sys32_iovec_t are not equivalent structures; iov array must be converted
		HeapBuffer<uapi::iovec> vector(iovcnt);
		for(int index = 0; index < iovcnt; index++) {

			vector[index].iov_base = reinterpret_cast<void*>(iov[index].iov_base);
			vector[index].iov_len = static_cast<uapi::size_t>(iov[index].iov_len);
		}

		return static_cast<sys32_long_t>(SystemCall::Invoke(sys_writev, reinterpret_cast<ContextHandle*>(context), fd, vector, iovcnt));
	}

	catch(...) { return static_cast<sys32_long_t>(SystemCall::TranslateException(std::current_exception())); }
}

#endif

#ifdef _M_X64
// sys64_writev
//
sys64_long_t sys64_writev(sys64_context_t context, sys64_int_t fd, sys64_iovec_t* iov, sys64_int_t iovcnt)
{
	static_assert(sizeof(uapi::iovec) == sizeof(sys64_iovec_t), "uapi::iovec is not equivalent to sys64_iovec_t");
	return SystemCall::Invoke(sys_writev, reinterpret_cast<ContextHandle*>(context), fd, reinterpret_cast<uapi::iovec*>(iov), iovcnt);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
