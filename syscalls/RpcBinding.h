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
#include "RpcBindingTemplate.h"			// Include RpcBindingTemplate declarations
#include "RpcException.h"				// Include RpcException declarations
#include "RpcProtocol.h"				// Include RpcProtocol declarations
#include "RpcString.h"					// Include RpcString declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcBinding
//
// Implements an RPC binding handle.  "Classic" handles are created by calling
// the various .Compose() static methods, whereas "Fast" handles are created 
// by calling the various .Create() static methods.  See RpcBindingCreate()
// in MSDN for the differences between classic and fast RPC binding handles

class RpcBinding
{
public:

	// Destructor
	//
	~RpcBinding() { if(m_handle) RpcBindingFree(&m_handle); }

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// RPC_BINDING_HANDLE()
	//
	operator RPC_BINDING_HANDLE() const { return m_handle; }

	//-------------------------------------------------------------------------
	// Member Functions

	// Bind
	//
	// Synchronously binds a "fast" RPC binding handle to the server
	void Bind(RPC_IF_HANDLE iface) const;

	// Compose
	// 
	// Composes a "classic" RPC binding from string components
	static RpcBinding* Compose(const RpcProtocol& protocol, const tchar_t* endpoint)
		{ return Compose(nullptr, protocol, endpoint, nullptr); }

	static RpcBinding* Compose(const RpcProtocol& protocol, const tchar_t* endpoint, const tchar_t* options)
		{ return Compose(nullptr, protocol, endpoint, options); }

	static RpcBinding* Compose(const tchar_t* server, const RpcProtocol& protocol, const tchar_t* endpoint)
		{ return Compose(server, protocol, endpoint, nullptr); }

	static RpcBinding* Compose(const tchar_t* server, const RpcProtocol& protocol, const tchar_t* endpoint, const tchar_t* options);

	// Copy
	//
	// Copies the specified binding handle into a new binding handle
	RpcBinding* Copy(void);

	// Create
	//
	// Creates a "fast" RPC binding from binding templates
	static RpcBinding* Create(RpcBindingTemplate& template_)
		{ return Create(template_, nullptr, nullptr); }

	static RpcBinding* Create(PRPC_BINDING_HANDLE_TEMPLATE_V1 template_)
		{ return Create(template_, nullptr, nullptr); }

	static RpcBinding* Create(PRPC_BINDING_HANDLE_TEMPLATE_V1 template_, PRPC_BINDING_HANDLE_SECURITY_V1 security)
		{ return Create(template_, security, nullptr); }

	static RpcBinding* Create(PRPC_BINDING_HANDLE_TEMPLATE_V1 template_, PRPC_BINDING_HANDLE_SECURITY_V1 security, PRPC_BINDING_HANDLE_OPTIONS_V1 options);

	// Reset
	//
	// Resets the binding handle by disassociating it from the server
	void Reset(void) const;

	// Unbind
	//
	// Unbinds the handle from the remote server, does not disconnect
	void Unbind(void) const;

	//-------------------------------------------------------------------------
	// Properties

	// Handle
	//
	// Gets the underlying RPC_BINDING_HANDLE
	__declspec(property(get=getHandle)) RPC_BINDING_HANDLE Handle;
	RPC_BINDING_HANDLE getHandle(void) const { return m_handle; }

	// Object
	//
	// Gets/sets the remote object UUID
	__declspec(property(get=getObject, put=putObject)) uuid_t Object;
	uuid_t getObject(void) const;
	void putObject(uuid_t value);

private:

	RpcBinding(const RpcBinding& rhs);
	RpcBinding& operator=(const RpcBinding& rhs);

	// Instance Constructor
	//
	explicit RpcBinding(RPC_BINDING_HANDLE handle) : m_handle(handle) {}

	//-------------------------------------------------------------------------
	// Member Variables

	RPC_BINDING_HANDLE		m_handle = nullptr;		// Contained binding handle
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCBINDING_H_
