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
#include "VmService.h"
#include <syscalls32.h>

#pragma warning(push, 4)

#ifdef _M_X64
#error syscalls32_32 module is not inteneded for use in 64-bit builds; use syscalls32_64
#endif

struct sys32_state_t
{
	VmService*		instance;
	// VmProcess*	process;
};

//-----------------------------------------------------------------------------
// acquire_context
//
// Creates a new context handle for a client process attaching to the interface
//
// Arguments:
//
//	rpchandle		- RPC binding handle
//	context			- [out] set to the newly allocated context handle

static sys32_long_t acquire_context(handle_t rpchandle, sys32_context_exclusive_t* context)
{
	// Allocate a new state object instance that will be case into the context handle
	sys32_state_t* state = reinterpret_cast<sys32_state_t*>(MIDL_user_allocate(sizeof(sys32_state_t)));
	if(!state) return -LINUX_ENOMEM;

	state->instance = 0;
	// state->process = 0;

	// Cast the state object into the client context handle pointer
	*context = reinterpret_cast<sys32_context_exclusive_t>(state);

	return 0;
}

//-----------------------------------------------------------------------------
// release_context
//
// Releases a context handle proviously allocated with allocate_context
//
// Arguments:
//
//	context			- [in/out] contains the handle to release and will be set to null

static sys32_long_t release_context(sys32_context_exclusive_t* context)
{
	// Cast the state object back from the context handle
	sys32_state_t* state = reinterpret_cast<sys32_state_t*>(*context);
	if(!state) return -LINUX_EFAULT;

	MIDL_user_free(state);				// Release the state structure
	*context = nullptr;					// Reset handle to null

	return 0;
}

//---------------------------------------------------------------------------
// sys32_context_exclusive_t_rundown
//
// Invoked by the RPC runtime when a client has disconnected without properly
// releasing an allocated context handle
//
// Arguments:
//
//	context			- Context handle to be forcibly released

void __RPC_USER sys32_context_exclusive_t_rundown(sys32_context_exclusive_t context)
{
	release_context(&context);
	_ASSERTE(context == nullptr);
}

//-----------------------------------------------------------------------------
// syscalls32_32
//
// 32-bit system calls object for use with 32-bit builds

SystemCalls32_v1_0_epv_t syscalls32_32 = {

	/* sys32_acquire_context = */	acquire_context,
	/* sys32_release_context = */	release_context,

	// 006: sys32_close
	[](sys32_context_t context, sys32_int_t fd) -> sys32_long_t 
	{ 
		sys32_state_t* state = reinterpret_cast<sys32_state_t*>(context);
		//return state->instance->blah(state->process, fd);
		return -1; 
	},
};

//---------------------------------------------------------------------------

#pragma warning(pop)
