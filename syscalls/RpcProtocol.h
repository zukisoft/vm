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

#ifndef __RPCPROTOCOL_H_
#define __RPCPROTOCOL_H_
#pragma once

#include "char_t.h"						// Include char_t declarations
#include "tstring.h"					// Include tstring<> declarations

#pragma warning(push, 4)				// Enable maximum compiler warnings

//-----------------------------------------------------------------------------
// RpcProtocol
//
// 

class RpcProtocol
{
public:

	//-------------------------------------------------------------------------
	// Overloaded Operators

	// const tchar_t*()
	//
	operator const tchar_t*() const { return m_value.c_str(); }

	//-------------------------------------------------------------------------
	// Fields

	// Http
	//
	// Internet Information Server protocol sequence
	static const RpcProtocol Http;

	// Local
	//
	// Local interprocess communication protocol sequence
	static const RpcProtocol Local;

	// NamedPipes
	//
	// Connection-oriented Named Pipes protocol sequence
	static const RpcProtocol NamedPipes;

	// TcpIp
	//
	// Connection-oriented TCP/IP protocol sequence
	static const RpcProtocol TcpIp;

private:

	RpcProtocol(const RpcProtocol& rhs);
	RpcProtocol& operator=(const RpcProtocol& rhs);

	// Instance Constructor
	//
	RpcProtocol(const tchar_t* protocol) : m_value(protocol) {}

	//-------------------------------------------------------------------------
	// Member Variables

	std::tstring		m_value;				// Contained string value
};

// RPCPROTOCOL STATIC MEMBER INITIALIZATIONS
//
__declspec(selectany) const RpcProtocol RpcProtocol::Http(_T("ncacn_http"));
__declspec(selectany) const RpcProtocol RpcProtocol::Local(_T("ncalrpc"));
__declspec(selectany) const RpcProtocol RpcProtocol::NamedPipes(_T("ncacn_np"));
__declspec(selectany) const RpcProtocol RpcProtocol::TcpIp(_T("ncacn_ip_tcp"));

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCPROTOCOL_H_
