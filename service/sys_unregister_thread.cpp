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
#include "SystemCall.h"			// TODO: REMOVE ME?

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// sys32_unregister_thread
//
// Unregisters a hosted thread from the Process container
//
// Arguments:
//
//	rpchandle		- RPC binding handle
//	nativetid		- Native thread id to be unregistered from the process
//	status			- Thread exit code

HRESULT sys32_unregister_thread(handle_t rpchandle, sys32_uint_t nativetid, sys32_int_t exitcode)
{
	(rpchandle);
	(nativetid);
	(exitcode);
	return S_OK;
}

//---------------------------------------------------------------------------

#pragma warning(pop)