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
	fshandle_t		fshandle;			// fshandle_t from remote services

	// Get a bound RPC handle for the remote system calls service
	handle_t rpc = rpc_bind_thread();
	if(rpc == nullptr) return -LINUX_EREMOTEIO;

	// Invoke the remote method call
	__int3264 result = rpc005_open(rpc, reinterpret_cast<charptr_t>(context->Ebx),
		static_cast<int32_t>(context->Ecx), static_cast<mode_t>(context->Edx), &fshandle);

	// do something with fshandle here

	return result;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
