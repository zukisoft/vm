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

#ifndef __RPCBINDINGTEMPLATE_H_
#define __RPCBINDINGTEMPLATE_H_
#pragma once

#include "RpcProtocol.h"				// Include RpcProtocol declarations
#include "RpcString.h"					// Include RpcString declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcBindingTemplate
//
// Defines a "fast" RPC binding handle template, used with RpcBinding<>

class RpcBindingTemplate
{
public:

	// Instance Constructors
	//
	RpcBindingTemplate(const RpcProtocol& protocol);
	RpcBindingTemplate(const RpcProtocol& protocol, const uuid_t& object);
	RpcBindingTemplate(const RpcProtocol& protocol, const tchar_t* endpoint);
	RpcBindingTemplate(const RpcProtocol& protocol, const tchar_t* endpoint, const uuid_t& object);
	RpcBindingTemplate(const tchar_t* server, const RpcProtocol& protocol);
	RpcBindingTemplate(const tchar_t* server, const RpcProtocol& protocol, const uuid_t& object);
	RpcBindingTemplate(const tchar_t* server, const RpcProtocol& protocol, const tchar_t* endpoint);
	RpcBindingTemplate(const tchar_t* server, const RpcProtocol& protocol, const tchar_t* endpoint, const uuid_t& object);

	// Copy Constructor
	//
	RpcBindingTemplate(const RpcBindingTemplate& rhs);

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// PRPC_BINDING_HANDLE_TEMPLATE_V1()
	//
	operator PRPC_BINDING_HANDLE_TEMPLATE_V1() { return &m_template; }

private:

	RpcBindingTemplate& operator=(const RpcBindingTemplate& rhs);

	//-------------------------------------------------------------------------
	// Member Variables

	RPC_BINDING_HANDLE_TEMPLATE_V1	m_template;		// Template structure
	RpcString						m_server;		// Target server name
	RpcString						m_endpoint;		// Endpoint string	
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCBINDINGTEMPLATE_H_
