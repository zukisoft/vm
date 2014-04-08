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

#include "stdafx.h"						// Include project pre-compiled headers
#include "uapi.h"						// Include Linux UAPI declarations

// remove me
#include <io.h>

#pragma warning(push, 4)				// Enable maximum compiler warnings

// rpc.cpp
handle_t rpc_bind_thread(void);

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
	fsobject_t			fsobject;			// fsobject_t from remote services

	// Get a bound RPC handle for the remote system calls service
	handle_t rpc = rpc_bind_thread();
	if(rpc == nullptr) return -LINUX_EREMOTEIO;

	// Structure must be initialized to zeros before invoking the remote method
	memset(&fsobject, 0x00, sizeof(fsobject_t));

	// Invoke the remote method to get information about the requested object
	__int3264 result = rpc005_open(rpc, reinterpret_cast<charptr_t>(context->Ebx),
		static_cast<int32_t>(context->Ecx), static_cast<mode_t>(context->Edx), &fsobject);
	if(result < 0) return result;

	///////// TESTING
	HANDLE test;

	switch(fsobject.objecttype) {

		case FSOBJECT_PHYSICAL:

			test = CreateFile(fsobject.physical.ospath, GENERIC_READ | GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
			if(test == INVALID_HANDLE_VALUE) {
				result = GetLastError();
			}

			midl_user_free(fsobject.physical.ospath);
			
			if(result != ERROR_SUCCESS) return -LINUX_EACCES;
			else return _open_osfhandle(uintptr_t(test), 0);

			break;
	};

	////////////////////////////////

	return -1;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
