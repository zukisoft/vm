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
// sys_statfs
//
// Gets mounted file system statistics
//
// Arguments:
//
//	context		- SystemCall context object
//	path		- Path to any object within the mounted file system
//	buf			- Output buffer

__int3264 sys_statfs(const SystemCall::Context* context, const uapi::char_t* path, uapi::statfs* buf)
{
	_ASSERTE(context);
	(buf);
	(path);
	return -LINUX_ENOSYS;
	//try { 

	//	SystemCall::Impersonation impersonation;

	//	// Get the Alias instance to use as the base for the path resolution operation
	//	auto alias = (fd == LINUX_AT_FDCWD) ? context->Process->WorkingDirectory : context->Process->GetHandle(fd)->Alias;

	//	// Attempt to open the file system object relative to the existing handle's alias
	//	return context->Process->AddHandle(context->VirtualMachine->OpenFile(alias, pathname, flags, mode));
	//}

	//catch(...) { return SystemCall::TranslateException(std::current_exception()); }
}

// sys32_statfs
//
sys32_long_t sys32_statfs(sys32_context_t context, const sys32_char_t* path, uapi::statfs32* buf)
{
	uapi::statfs		stats;				// Generic statfs structure (64-bit fields)

	// Invoke the generic version of the system call using the local structure
	sys32_long_t result = static_cast<sys32_long_t>(sys_statfs(reinterpret_cast<SystemCall::Context*>(context), path, &stats));

	// If sys_statfs() was successful, convert the data from the generic structure into the compatible one
	if(result >= 0) {

		buf->f_type		= static_cast<uint32_t>(stats.f_type);
		buf->f_bsize	= static_cast<uint32_t>(stats.f_bsize);
		buf->f_blocks	= static_cast<uint32_t>(stats.f_blocks);
		buf->f_bfree	= static_cast<uint32_t>(stats.f_bfree);
		buf->f_bavail	= static_cast<uint32_t>(stats.f_bavail);
		buf->f_files	= static_cast<uint32_t>(stats.f_files);
		buf->f_ffree	= static_cast<uint32_t>(stats.f_ffree);
		buf->f_fsid		= stats.f_fsid;
		buf->f_namelen	= static_cast<uint32_t>(stats.f_namelen);
		buf->f_frsize	= static_cast<uint32_t>(stats.f_frsize);
		buf->f_flags	= static_cast<uint32_t>(stats.f_flags);
		buf->f_spare[0] = static_cast<uint32_t>(stats.f_spare[0]);
		buf->f_spare[1] = static_cast<uint32_t>(stats.f_spare[1]);
		buf->f_spare[2] = static_cast<uint32_t>(stats.f_spare[2]);
		buf->f_spare[3] = static_cast<uint32_t>(stats.f_spare[3]);
	}

	return result;
}

#ifdef _M_X64
// sys64_statfs
//
sys64_long_t sys64_statfs(sys64_context_t context, const sys64_char_t* path, uapi::statfs* buf)
{
	return sys_statfs(reinterpret_cast<SystemCall::Context*>(context), path, buf);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
