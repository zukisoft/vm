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
#include "PathSplitter.h"
#include "SystemCall.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys_mknodat
//
// Creates a special file or device node
//
// Arguments:
//
//	context		- SystemCall context object
//	dirfd		- Previously opened directory object file descriptor
//	pathname	- Relative path for the directory to create
//	mode		- Mode flags to assign when creating the directory
//	device		- Device identifier for creation of a device node

__int3264 sys_mknodat(const SystemCall::Context* context, int dirfd, const uapi::char_t* pathname, uapi::mode_t mode, uapi::dev_t device)
{
	_ASSERTE(context);

	try {

		SystemCall::Impersonation impersonation;

		// Determine if an absolute or relative pathname has been provided
		bool absolute = ((pathname) && (pathname[0] == '/'));

		// Determine the base alias from which to resolve the path
		FileSystem::AliasPtr base = absolute ? context->Process->RootDirectory : 
			((dirfd == LINUX_AT_FDCWD) ? context->Process->WorkingDirectory : context->Process->GetHandle(dirfd)->Alias);

		// Apply the process' current umask to the provided creation mode flags
		mode &= ~context->Process->FileCreationModeMask;

		// Invoke the proper method on the virtual machine based on the node type requested
		switch(mode & LINUX_S_IFMT) {

			// S_IFREG (or zero): Create a new regular file node
			case 0:
			case LINUX_S_IFREG:
				_RPTF0(_CRT_ASSERT, "mknodat: S_IFREG not implemented yet");
				break;

			// S_IFCHR: Create a new character device node
			case LINUX_S_IFCHR:
				context->VirtualMachine->CreateCharacterDevice(context->Process->RootDirectory, base, pathname, mode, device);
				break;

			// S_IFBLK: Create a new block device node
			case LINUX_S_IFBLK:
				_RPTF0(_CRT_ASSERT, "mknodat: S_IFBLK not implemented yet");
				break;

			// S_IFIFO: Create a new pipe node
			case LINUX_S_IFIFO:
				_RPTF0(_CRT_ASSERT, "mknodat: S_IFIFO not implemented yet");
				break;

			// S_IFSOCK: Create a new socket node
			case LINUX_S_IFSOCK:
				_RPTF0(_CRT_ASSERT, "mknodat: S_IFSOCK not implemented yet");
				break;

			// Other node types cannot be created by this system call
			default: throw LinuxException(LINUX_EINVAL);
		}
	}

	catch(...) { return SystemCall::TranslateException(std::current_exception()); }

	return 0;
}

// sys32_mknodat
//
sys32_long_t sys32_mknodat(sys32_context_t context, sys32_int_t dirfd, const sys32_char_t* pathname, sys32_mode_t mode, sys32_dev_t device)
{
	return static_cast<sys32_long_t>(sys_mknodat(reinterpret_cast<SystemCall::Context*>(context), dirfd, pathname, mode, device));
}

#ifdef _M_X64
// sys64_mknodat
//
sys64_long_t sys64_mknodat(sys64_context_t context, sys64_int_t dirfd, const sys64_char_t* pathname, sys64_mode_t mode, sys64_dev_t device)
{
	return sys_mknodat(reinterpret_cast<SystemCall::Context*>(context), dirfd, pathname, mode, device);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
