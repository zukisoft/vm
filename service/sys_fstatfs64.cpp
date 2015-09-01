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

// sys_fstatfs.cpp
uapi::long_t sys_fstatfs(const Context* context, int fd, uapi::statfs* buf);

// sys32_fstatfs64
//
sys32_long_t sys32_fstatfs64(sys32_context_t context, sys32_int_t fd, sys32_size_t length, linux_statfs3264* buf)
{
	return -LINUX_ENOSYS;

	//uapi::statfs		stats;				// Generic statfs structure (64-bit fields)

	//// Only the statfs3264 structure is supported by this system call
	//if(buf == nullptr) return -LINUX_EFAULT;
	//if(length != sizeof(uapi::statfs3264)) return -LINUX_EFAULT;

	//// Invoke the generic version of the system call using the local structure
	//sys32_long_t result = static_cast<sys32_long_t>(SystemCall::Invoke(sys_fstatfs, context, fd, &stats));

	//// If sys_fstatfs() was successful, convert the data from the generic structure into the compatible one
	//if(result >= 0) {

	//	buf->f_type		= static_cast<int32_t>(stats.f_type);
	//	buf->f_bsize	= static_cast<int32_t>(stats.f_bsize);
	//	buf->f_blocks	= stats.f_blocks;
	//	buf->f_bfree	= stats.f_bfree;
	//	buf->f_bavail	= stats.f_bavail;
	//	buf->f_files	= stats.f_files;
	//	buf->f_ffree	= stats.f_ffree;
	//	buf->f_fsid		= stats.f_fsid;
	//	buf->f_namelen	= static_cast<int32_t>(stats.f_namelen);
	//	buf->f_frsize	= static_cast<int32_t>(stats.f_frsize);
	//	buf->f_flags	= static_cast<int32_t>(stats.f_flags);
	//	buf->f_spare[0] = static_cast<int32_t>(stats.f_spare[0]);
	//	buf->f_spare[1] = static_cast<int32_t>(stats.f_spare[1]);
	//	buf->f_spare[2] = static_cast<int32_t>(stats.f_spare[2]);
	//	buf->f_spare[3] = static_cast<int32_t>(stats.f_spare[3]);
	//}

	//return result;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
