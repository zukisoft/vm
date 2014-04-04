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

#ifndef __RPCSTRING_H_
#define __RPCSTRING_H_
#pragma once

#pragma warning(push, 4)			// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// Type Declarations

// RPC_TSTR/RPC_CTSTR
//
// Primitive generic-text RPC string pointer types
#ifdef _UNICODE
typedef unsigned short		RPC_TCHAR;
typedef RPC_WSTR			RPC_TSTR;
typedef RPC_CWSTR			RPC_CTSTR;
#else
typedef unsigned char		RPC_TCHAR;
typedef unsigned char*		RPC_TSTR;
typedef RPC_CSTR			RPC_CTSTR;
#endif

//-----------------------------------------------------------------------------
// Class RpcString

class RpcString
{
public:
	
	// Instance Constructors
	//
	RpcString() {}
	RpcString(RPC_CTSTR str) { operator=(str); }
	RpcString(const tchar_t* str) { operator=(str); }
	RpcString(const RpcString& rhs) { operator=(rhs); }

	// Destructor
	//
	~RpcString() { Release(); }
	
	// Assignment Operators
	//
	RpcString& operator=(RPC_CTSTR rhs);
	RpcString& operator=(const tchar_t* rhs) { return operator=(reinterpret_cast<RPC_CTSTR>(rhs)); }
	RpcString& operator=(const RpcString& rhs) { return operator=(rhs.m_string); }

	// Boolean Conversion Operators
	//
	operator bool() const { return m_string != nullptr; }
	bool operator !() const { return m_string == nullptr; }

	// String Pointer Conversion Operators
	//
	operator RPC_TSTR() const { return m_string; }
	operator RPC_CTSTR() const { return m_string; }
	operator tchar_t*() const { return reinterpret_cast<tchar_t*>(m_string); }
	operator const tchar_t*() const { return reinterpret_cast<tchar_t*>(m_string); }

private:

	// Address-of Operator
	// TODO: Expect to use this for RPC_TSTR* output strings; may not
	//
	// RPC_TSTR* operator &();

	//-------------------------------------------------------------------------
	// Private Member Functions

	tchar_t* Alloc(size_t length);
	void Release(void);

	//-------------------------------------------------------------------------
	// Member Variables

	RPC_TSTR		m_string = nullptr;			// Pointer to the string data
	bool			m_rpcfree = false;			// Flag if RpcStringFree() needed
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCSTRING_H_
