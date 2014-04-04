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
#include "RpcBindingTemplate.h"			// Include RpcBindingTemplate declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcBindingTemplate Constructor
//
// Arguments:
//
//	protocol	- Protocol sequence

RpcBindingTemplate::RpcBindingTemplate(const RpcProtocol& protocol) : 
	RpcBindingTemplate(nullptr, protocol, nullptr) {}

//-----------------------------------------------------------------------------
// RpcBindingTemplate Constructor
//
// Arguments:
//
//	protocol	- Protocol sequence
//	endpoint	- Endpoint; format depends on protocol sequence

RpcBindingTemplate::RpcBindingTemplate(const RpcProtocol& protocol, const tchar_t* endpoint) : 
	RpcBindingTemplate(nullptr, protocol, endpoint) {}

//-----------------------------------------------------------------------------
// RpcBindingTemplate Constructor
//
// Arguments:
//
//	server		- Target server; format depends on protocol sequence
//	protocol	- Protocol sequence

RpcBindingTemplate::RpcBindingTemplate(const tchar_t* server, const RpcProtocol& protocol) : 
	RpcBindingTemplate(server, protocol, nullptr) {}

//-----------------------------------------------------------------------------
// RpcBindingTemplate Constructor
//
// Arguments:
//
//	server		- Target server; format depends on protocol sequence
//	protocol	- Protocol sequence
//	endpoint	- Endpoint; format depends on protocol sequence

RpcBindingTemplate::RpcBindingTemplate(const tchar_t* server, const RpcProtocol& protocol,
	const tchar_t* endpoint) : m_server(server), m_endpoint(endpoint)
{
	memset(&m_template, 0, sizeof(RPC_BINDING_HANDLE_TEMPLATE_V1));

	// Initialize the member structure, use pointers to the contained strings
	m_template.Version = 1;
	m_template.Flags = 0;
	m_template.ProtocolSequence = protocol;
	m_template.NetworkAddress = m_server;
	m_template.StringEndpoint = m_endpoint;
	m_template.ObjectUuid = GUID_NULL;
}

//-----------------------------------------------------------------------------
// RpcBindingTemplate Copy Constructor

RpcBindingTemplate::RpcBindingTemplate(const RpcBindingTemplate& rhs)
	: m_server(rhs.m_server), m_endpoint(rhs.m_endpoint)
{
	// Copy the structure over and point the string members to our copies
	memcpy(&m_template, &rhs.m_template, sizeof(RPC_BINDING_HANDLE_TEMPLATE_V1));
	m_template.NetworkAddress = m_server;
	m_template.StringEndpoint = m_endpoint;
}

//-----------------------------------------------------------------------------

#pragma warning(pop)
