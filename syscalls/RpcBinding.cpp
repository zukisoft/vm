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
#include "RpcBinding.h"					// Include RpcBinding declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcBinding::Bind
//
// Synchronously (re)binds a fast RPC binding handle to the interface
//
// Arguments:
//
//	iface		- Remote server interface to bind to the handle

void RpcBinding::Bind(RPC_IF_HANDLE iface) const
{
	RPC_STATUS result = RpcBindingBind(nullptr, m_handle, iface);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------
// RpcBinding::Compose (static)
//
// Composes a classic RPC binding from component parts
//
// Arguments:
//
//	server		- Target server; format depends on protocol sequence
//	protocol	- Protocol sequence
//	endpoint	- Endpoint; format depends on protocol sequence
//	options		- Binding options; format depends on protocol sequence

RpcBinding* RpcBinding::Compose(const tchar_t* server, const RpcProtocol& protocol, 
	const tchar_t* endpoint, const tchar_t* options)
{
	RPC_TSTR			composed;			// Composed RPC binding string
	RPC_BINDING_HANDLE	handle;				// The new RPC binding handle
	RPC_STATUS			result;				// Result from RPC function call

	// Attempt to compose a binding string from the component parts (UUID is not
	// currently supported, but would be easy to add later)
	result = RpcStringBindingCompose(nullptr, RpcString(protocol), RpcString(server),
		RpcString(endpoint), RpcString(options), &composed);
	if(result != RPC_S_OK) throw RpcException(result);

	// Attempt to convert the composed binding string into a binding handle
	result = RpcBindingFromStringBinding(composed, &handle);

	// Release the composed binding string and throw if binding wasn't created
	RpcStringFree(&composed);
	if(result != RPC_S_OK) throw RpcException(result);

	return new RpcBinding(handle);
}

//-----------------------------------------------------------------------------
// RpcBinding::Copy
//
// Creates a copy of the original RpcBinding handle
//
// Arguments:
//
//	rhs			- RpcBinding instance to copy

RpcBinding* RpcBinding::Copy(void)
{
	RPC_BINDING_HANDLE		handle;			// New RPC binding handle
	
	RPC_STATUS result = RpcBindingCopy(m_handle, &handle);
	if(result != RPC_S_OK) throw RpcException(result);

	return new RpcBinding(handle);
}

//-----------------------------------------------------------------------------
// RpcBinding::Create (static)
//
// Creates a template-based fast RPC binding
//
// Arguments:
//
//	template_	- Binding handle template
//	security	- Binding handle security options
//	options		- Binding handle options

RpcBinding* RpcBinding::Create(PRPC_BINDING_HANDLE_TEMPLATE_V1 template_, PRPC_BINDING_HANDLE_SECURITY_V1 security, 
	PRPC_BINDING_HANDLE_OPTIONS_V1 options)
{
	RPC_BINDING_HANDLE	handle;				// The new RPC binding handle
	RPC_STATUS			result;				// Result from RPC function call

	result = RpcBindingCreate(template_, security, options, &handle);
	if(result != RPC_S_OK) throw RpcException(result);
	
	return new RpcBinding(handle);
}

//-----------------------------------------------------------------------------
// RpcBinding::getObject
//
// Gets the object UUID for the binding handle

uuid_t RpcBinding::getObject(void) const
{
	uuid_t		value;				// Value returned from RPC call

	RPC_STATUS result = RpcBindingInqObject(m_handle, &value);
	if(result != RPC_S_OK) throw RpcException(result);

	return value;
}

//-----------------------------------------------------------------------------
// RpcBinding::putObject
//
// Sets the object UUID for the binding handle

void RpcBinding::putObject(uuid_t value)
{
	//uuid_t object = value;			// Copy into a non-const UUID

	RPC_STATUS result = RpcBindingSetObject(m_handle, &value);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------
// RpcBinding::Reset
//
// Resets the binding handle by disassociating it from the server
//
// Arguments:
//
//	NONE

void RpcBinding::Reset(void) const
{
	RPC_STATUS result = RpcBindingReset(m_handle);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------
// RpcBinding::Unbind
//
// Unbinds the handle from the remote server
//
// Arguments:
//
//	NONE

void RpcBinding::Unbind(void) const
{
	RPC_STATUS result = RpcBindingUnbind(m_handle);
	if(result != RPC_S_OK) throw RpcException(result);
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
