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

// t_gs (emulator.cpp)
//
// Emulated GS register value
extern __declspec(thread) uint16_t t_gs;

//-----------------------------------------------------------------------------
// sys_execve
//
// Executes a program
//
// Arguments:
//
//	context		- Pointer to the CONTEXT structure from the exception handler

uapi::long_t sys_execve(PCONTEXT context)
{
	int				argc = 0;		// Number of command-line argument strings
	int				envc = 0;		// Number of environment variables
	sys32_task_t	task;			// New task information after execve completes

	// These structures must be the same for this to work
	static_assert(sizeof(CONTEXT) == sizeof(sys32_task_t), "size of native CONTEXT differs from sys32_task_t");

	// Cast out the arguments to sys_execve
	const uapi::char_t*		filename	= reinterpret_cast<const uapi::char_t*>(context->Ebx);
	const uapi::char_t**	argv		= reinterpret_cast<const uapi::char_t**>(context->Ecx);
	const uapi::char_t**	envp		= reinterpret_cast<const uapi::char_t**>(context->Edx);

	// RPC doesn't have the capability to marshal multi-dimensional arrays, so
	// they have to be scanned to get the counts
	for(const uapi::char_t** element = argv; *element; element++) argc++;
	for(const uapi::char_t** element = envp; *element; element++) envc++;

	// Execute the RPC function with the calculated argument and envvar count
	uapi::long_t result = sys32_execve(t_rpccontext, filename, argc, argv, envc, envp, &task);
	if(result != 0) return result;

	// Selectively apply the new context registers here
	// context->Eax = result;
	context->Ebx = task.ebx;
	context->Ecx = task.ecx;
	context->Edx = task.edx;
	// more, obviously
	context->Eip = task.eip;
	context->Esp = task.esp;
	context->Ebp = task.ebp;

	// floating point state?

	t_gs = 0;

	return result;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)


