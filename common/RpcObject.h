//-----------------------------------------------------------------------------
// Copyright (c) 2015 Michael G. Brehm
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

#ifndef __RPCOBJECT_H_
#define __RPCOBJECT_H_
#pragma once

#include <memory>
#include "generic_text.h"
#include "Win32Exception.h"

#pragma warning(push, 4)
#pragma warning(disable:4396)	// inline specifier cannot be used with specialization

//-----------------------------------------------------------------------------
// RpcObject
//
// Specialized RPC interface registration wrapper used for implementing multiple
// instances of that interface within the same process

class RpcObject
{
public:

	// Destructor
	//
	~RpcObject();

	//-------------------------------------------------------------------------
	// Member Functions

	// Create (static)
	//
	// Creates a new RPC object instance
	static std::unique_ptr<RpcObject> Create(const RPC_IF_HANDLE& ifspec, unsigned int flags);
	static std::unique_ptr<RpcObject> Create(const RPC_IF_HANDLE& ifspec, const uuid_t& objectid, unsigned int flags);
	static std::unique_ptr<RpcObject> Create(const RPC_IF_HANDLE& ifspec, const uuid_t& objectid, const uuid_t& mgrtypeid, unsigned int flags);
	static std::unique_ptr<RpcObject> Create(const RPC_IF_HANDLE& ifspec, const uuid_t& objectid, const uuid_t& mgrtypeid, RPC_MGR_EPV* const epv, unsigned int flags);

	//-------------------------------------------------------------------------
	// Properties

	// BindingString
	//
	// Gets the binding string required for a client to connect to the object
	__declspec(property(get=getBindingString)) const tchar_t* BindingString;
	const tchar_t* getBindingString(void) const;

	// ObjectId
	//
	// Unique identifer for the constructed RPC object
	__declspec(property(get=getObjectId)) uuid_t ObjectId;
	uuid_t getObjectId(void) const;

private:

	RpcObject(const RpcObject &rhs)=delete;
	RpcObject& operator=(const RpcObject &rhs)=delete;

	// Instance Constructor
	//
	RpcObject(const RPC_IF_HANDLE& ifspec, const uuid_t& objectid, const uuid_t& mgrtypeid, std::tstring&& bindingstr);
	friend std::unique_ptr<RpcObject> std::make_unique<RpcObject, const RPC_IF_HANDLE&, const uuid_t&, const uuid_t&, std::tstring>(const RPC_IF_HANDLE&, const uuid_t&, const uuid_t&, std::tstring&&);

	//-------------------------------------------------------------------------
	// Private Member Functions

	// AddObjectMapping (static)
	//
	// Adds an object type mapping to a registered RPC interface
	static std::tstring AddObjectMapping(const RPC_IF_HANDLE& ifspec, const uuid_t& objectid, const uuid_t& mgrtypeid);

	// RemoveObjectMapping (static)
	//
	// Removes an object type mapping from a registered RPC interface
	static void RemoveObjectMapping(const RPC_IF_HANDLE& ifspec, const uuid_t& objectid);

	//-------------------------------------------------------------------------
	// Member Variables

	bool					m_revoked = false;		// Flag if object was revoked
	const RPC_IF_HANDLE		m_ifspec;				// Interface specification
	const uuid_t			m_objectid;				// Object unique identifier
	const uuid_t			m_mgrtypeid;			// Entry point vector manager type
	const std::tstring		m_bindingstr;			// Binding string for the object
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCOBJECT_H_
