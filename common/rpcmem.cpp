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

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// midl_user_allocate
//
// Allocates RPC stub and library memory
//
// Arguments:
//
//	len			- Length of the required memory buffer

void __RPC_FAR * __RPC_USER midl_user_allocate(size_t len)
{
	// Use the COM task memory allocator for RPC
	return CoTaskMemAlloc(len);
}

//-----------------------------------------------------------------------------
// midl_user_free
//
// Relases RPC stub and library memory
//
// Arguments:
//
//	ptr			- Pointer to buffer allocated by MIDL_user_allocate

void __RPC_USER midl_user_free(void __RPC_FAR* ptr)
{
	// Use the COM task memory allocator for RPC
	CoTaskMemFree(ptr);
}

//---------------------------------------------------------------------------

#pragma warning(pop)