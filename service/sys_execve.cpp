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

//-----------------------------------------------------------------------------
// sys_execve
//
// Executes a program
//
// Arguments:
//
//	context			- System call context object
//	filename		- Path to the binary file to be executed
//	argv			- Command-line argument array
//	envp			- Environment variable array

uapi::long_t sys_execve(const Context* context, const uapi::char_t* filename, const uapi::char_t** argv, const uapi::char_t** envp)
{
	// Replace the process with the specified image
	context->Process->Execute(filename, argv, envp);
	return 0;
}

// sys32_execve
//
sys32_long_t sys32_execve(sys32_context_t context, const sys32_char_t* filename, sys32_int_t argc, const sys32_char_t* argv[], sys32_int_t envc, const sys32_char_t* envp[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(envc);

	// argc and envp are for RPC only, don't pass onto the system call
	return static_cast<sys32_long_t>(SystemCall::Invoke(sys_execve, context, filename, argv, envp));
}

#ifdef _M_X64
// sys64_execve
//
sys64_long_t sys64_execve(sys64_context_t context, const sys64_char_t* filename, sys64_int_t argc, const sys64_char_t* argv[], sys64_int_t envc, const sys64_char_t* envp[])
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(envc);

	// argc and envc are for RPC only, don't pass onto the system call
	return SystemCall::Invoke(sys_execve, context, filename, argv, envp);
}
#endif

//---------------------------------------------------------------------------

#pragma warning(pop)
