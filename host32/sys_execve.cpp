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

#pragma warning(push, 4)

// t_rpccontext (main.cpp)
//
// RPC context handle for the current thread
extern __declspec(thread) sys32_context_t t_rpccontext;

//-----------------------------------------------------------------------------
// sys_execve
//
// Executes a program
//
// Arguments:
//
//	filename		- Path to the binary file to be executed
//	argv			- Command-line argument array
//	envp			- Environment variable array

uapi::long_t sys_execve(const uapi::char_t* filename, const uapi::char_t* argv[], const uapi::char_t* envp[])
{
	int		argc = 1;		// Number of command-line argument strings (includes NULL)
	int		envc = 1;		// Number of environment variables (includes NULL)

	// RPC doesn't have the capability to marshal multi-dimensional arrays, so
	// they have to be scanned to get the element counts.  Include the final null
	// string as part of the count so that is also sent to the server

	for(const uapi::char_t** element = argv; *element; element++, argc++);
	for(const uapi::char_t** element = envp; *element; element++, envc++);

	// Execute the RPC function with the calculated argument and envvar count
	return sys32_execve(t_rpccontext, filename, argc, argv, envc, envp);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


