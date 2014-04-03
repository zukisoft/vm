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
#include "RpcString.h"					// Include RpcString declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcString::operator=

RpcString& RpcString::operator=(RPC_CTSTR str)
{
	Release();							// Release existing string
	
	// An RPC string is essentially just a typical C-style string
	const tchar_t* string = reinterpret_cast<const tchar_t*>(str);

	if(string) {

		// Reallocate the contained buffer and copy the string
		size_t length = _tcslen(string) + 1;
		_tcscpy_s(Alloc(length), length, string);
	}

	return *this;
}

//-----------------------------------------------------------------------------
// RpcString::Alloc (private)
//
// Allocates the underlying string buffer
//
// Arguments:
//
//	length			- Required buffer length, in RPC_TCHARs

tchar_t* RpcString::Alloc(size_t length)
{
	if(m_string) Release();				// Release previously allocated string

	// Allocate the new string buffer from the process heap
	m_string = new RPC_TCHAR[length];
	m_rpcfree = false;

	// For convenience the underlying buffer is returned as a tchar_t*
	return reinterpret_cast<tchar_t*>(m_string);
}

//-----------------------------------------------------------------------------
// RpcString::Release (private)
//
// Releases the underlying string buffer, which may have come from the CRT or
// the RPC runtime
//
// Arguments:
//
//	NONE

void RpcString::Release(void)
{
	if(m_rpcfree && m_string) RpcStringFree(&m_string);
	else if(m_string) delete[] m_string;

	m_string = nullptr;
	m_rpcfree = false;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
