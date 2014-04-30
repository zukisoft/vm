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
#include <linux/fcntl.h>

// remove me
#include <io.h>
#include "FsObject.h"
#include "FileDescriptorTable.h"

#pragma warning(push, 4)				// Enable maximum compiler warnings

// rpc.cpp
handle_t rpc_bind_thread(void);

#include <fcntl.h>

//
//
// Attempts to open a physical file system object
//
// Arguments:
//
//	object			- FsObject data returned from remote services
//	flags			- Flags passed into open()
//	mode			- Mode passed into open()
//
int open_physical(const FsObject& object, int flags, uapi::mode_t mode)
{
	DWORD		access;							// Win32 access mask
	DWORD		share = 0;						// Win32 share mask
	DWORD		disposition;					// Win32 disposition mask
	DWORD		attributes = 0;					// Win32 attributes mask

	UNREFERENCED_PARAMETER(mode);

	// TODO
	//SECURITY_ATTRIBUTES securityattrs;
	//securityattrs.nLength = sizeof(SECURITY_ATTRIBUTES);
	//securityattrs.lpSecurityDescriptor = nullptr;
	//securityattrs.bInheritHandle = TRUE;

	//const wchar_t* path = object.physical.ospath;

	// Convert the Linux flags into a Windows access mask; this must exist
	switch(flags & LINUX_O_ACCMODE) {

		case LINUX_O_RDONLY: access = GENERIC_READ | GENERIC_EXECUTE; break;
		case LINUX_O_WRONLY: access = GENERIC_WRITE | GENERIC_EXECUTE; break;
		case LINUX_O_RDWR: access = GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE; break;
		default: return -LINUX_EINVAL;
	}

	// Convert linux flags into Windows disposition mask
	switch(flags & (LINUX_O_CREAT | LINUX_O_EXCL | LINUX_O_TRUNC)) {
	
		case 0:
		case LINUX_O_EXCL: disposition = OPEN_EXISTING; break;

		case LINUX_O_CREAT: disposition = OPEN_ALWAYS; break;

		case LINUX_O_CREAT | LINUX_O_EXCL:
		case LINUX_O_CREAT | LINUX_O_TRUNC | LINUX_O_EXCL: disposition = CREATE_NEW; break;

		case LINUX_O_TRUNC:
		case LINUX_O_TRUNC | LINUX_O_EXCL: disposition = TRUNCATE_EXISTING; break;

		case LINUX_O_CREAT | LINUX_O_TRUNC: disposition = CREATE_ALWAYS; break;

		default: return -LINUX_EINVAL;			// <-- can't actually happen
	}
	
	// TODO: This is not even close to being done -- just enough to test read() and close() with

	// Attempt to open/create the physical file system object
	HANDLE handle = CreateFile(object.physical.ospath, access, share, nullptr, disposition, attributes, nullptr);
	if(handle == INVALID_HANDLE_VALUE) {
		
		// TODO: need all the errors here
		return -LINUX_ENOENT;
	}

	// Object handle has been opened, allocate the file descriptor entry
	return FileDescriptorTable::Allocate(object, handle);
}

// int open(const char* pathname, int flags, mode_t mode);
//
// EBX	- const char*	pathname
// ECX	- int			flags
// EDX	- mode_t		mode
// ESI
// EDI
// EBP
//
int sys005_open(PCONTEXT context)
{
	_ASSERTE(context->Eax == 5);				// Verify system call number

	FsObject				fsobject;				// fsobject_t from remote
	int						flags;					// Flags argument value
	uapi::mode_t			mode;					// Mode argument value

	// Get a bound RPC handle for the remote system calls service
	handle_t rpc = rpc_bind_thread();
	if(rpc == nullptr) return -LINUX_EREMOTEIO;
	
	// Pull out the flags and mode parameters from the CONTEXT object
	flags = static_cast<int>(context->Ecx);
	mode = static_cast<uapi::mode_t>(context->Edx);

	// Invoke the remote method to get information about the requested object
	__int3264 result = rpc005_open(rpc, reinterpret_cast<charptr_t>(context->Ebx), flags, mode, &fsobject);
	if(result < 0) return result;

	// Physical file system objects are handled in-process, virtual ones are handled remotely ...
	if(fsobject.objecttype == FSOBJECT_PHYSICAL) return open_physical(fsobject, flags, mode);
	else return FileDescriptorTable::Allocate(fsobject);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
