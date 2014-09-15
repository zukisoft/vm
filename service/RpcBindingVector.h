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

#ifndef __RPCBINDINGVECTOR_H_
#define __RPCBINDINGVECTOR_H_
#pragma once

#include "Exception.h"

#pragma warning(push, 4)			

//---------------------------------------------------------------------------
// Class RpcBindingVector
//
// Wraps a call to RpcServerInqBindings and provides access to the contained
// binding handles

class RpcBindingVector
{
public:

	// Constructor / Destructor
	//
	RpcBindingVector();
	~RpcBindingVector();

	// RPC_BINDING_VECTOR* conversion operator
	//
	operator RPC_BINDING_VECTOR*() const { return m_vector; }

	// Count
	//
	// Gets the number of available binding handles
	__declspec(property(get=getCount)) size_t Count;
	size_t getCount(void) const { return m_vector->Count; }

	// Handle[]
	//
	// Accesses the RPC_BINDING_HANDLE at the specified index
	__declspec(property(get=getHandle)) RPC_BINDING_HANDLE Handle[];
	RPC_BINDING_HANDLE getHandle(size_t index) const
	{
		if(index >= m_vector->Count) throw Exception(E_INVALIDARG);
		return m_vector->BindingH[index];
	}

private:

	RpcBindingVector(const RpcBindingVector &rhs)=delete;
	RpcBindingVector& operator=(const RpcBindingVector &rhs)=delete;

	//-------------------------------------------------------------------------
	// Member Variables

	RPC_BINDING_VECTOR*		m_vector = nullptr;		// Binding vector
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCBINDINGVECTOR_H_
