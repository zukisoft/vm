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

#ifndef __RPCBINDING_H_
#define __RPCBINDING_H_
#pragma once

#include "char_t.h"						// Include char_t declarations
#include "RpcException.h"				// Include RpcException declarations
#include "RpcProtocol.h"				// Include RpcProtocol declarations
#include "RpcString.h"					// Include RpcString declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcBinding
//
// 

class RpcBinding
{
public:

	// Destructor
	//
	~RpcBinding();

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// RPC_BINDING_HANDLE()
	//
	operator RPC_BINDING_HANDLE() const { return m_handle; }

	//-------------------------------------------------------------------------
	// Member Functions

	// Compose
	// 
	// Composes an RPC binding from string components
	static RpcBinding* Compose(const RpcProtocol& protocol, const tchar_t* endpoint)
		{ return Compose(nullptr, protocol, endpoint, nullptr); }

	static RpcBinding* Compose(const RpcProtocol& protocol, const tchar_t* endpoint, const tchar_t* options)
		{ return Compose(nullptr, protocol, endpoint, options); }

	static RpcBinding* Compose(const tchar_t* server, const RpcProtocol& protocol, const tchar_t* endpoint)
		{ return Compose(server, protocol, endpoint, nullptr); }

	static RpcBinding* Compose(const tchar_t* server, const RpcProtocol& protocol, const tchar_t* endpoint, const tchar_t* options);

	//-------------------------------------------------------------------------
	// Properties

	__declspec(property(get=getHandle)) RPC_BINDING_HANDLE Handle;
	RPC_BINDING_HANDLE getHandle(void) const { return m_handle; }

private:

	RpcBinding(const RpcBinding& rhs);
	RpcBinding& operator=(const RpcBinding& rhs);

	// Instance Constructor
	//
	explicit RpcBinding(RPC_BINDING_HANDLE handle);

	//-------------------------------------------------------------------------
	// Member Variables

	RPC_BINDING_HANDLE		m_handle = nullptr;		// Contained binding handle
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCBINDING_H_
