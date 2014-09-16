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

#ifndef __RPCAUTOLISTENER_H_
#define __RPCAUTOLISTENER_H_
#pragma once

#include <memory>
#include "Win32Exception.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// Class RpcAutoListener
//
// Implements a basic RPC listener for a single interface that applies the flag 
// RPC_IF_AUTOLISTEN during registration and registers an endpoint for all currently
// available protocol sequences.
//
// RPC constructs are intrinsically weakly typed, so pointers to the elements generated
// by MIDL as template arguments are (currently until C++14?) necessary

template <RPC_IF_HANDLE* ifspec, uuid_t* mgrtypeid = nullptr, RPC_MGR_EPV* mgrepv = nullptr>
class RpcAutoListener
{
public:

	// Destructor
	//
	~RpcAutoListener()
	{
		// Stop and unregister the interface/endpoint if it's running.  It's 
		// never a good plan to throw an exception from a destructor so eat them
		try { Stop(true); }
		catch(...) { /* DO NOTHING */ }

		// Release the bindings vector generated during Start
		if(m_bindings) RpcBindingVectorFree(&m_bindings);
	}

	//-------------------------------------------------------------------------
	// Member Functions

	// Start (static)
	//
	// Registers and starts the RPC interface/endpoint listener
	static std::unique_ptr<RpcAutoListener> Start(void)
	{
		uuid_t niluuid;
		UuidCreateNil(&niluuid);
		
		// Invoke StartObject() using a nil UUID to indicate there is no object
		return StartObject(niluuid);
	}

	// StartObject (static)
	//
	// Registers and starts the RPC interface/endpoint with an object identifier
	static std::unique_ptr<RpcAutoListener> StartObject(const uuid_t& objectid)
	{
		RPC_BINDING_VECTOR*		bindings;		// RPC server binding vector
		RPC_STATUS				rpcresult;		// Result from RPC function call

		// Retrieve the current set of server protocol sequence binding handles
		rpcresult = RpcServerInqBindings(&bindings);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);
		if(bindings->Count == 0) { RpcBindingVectorFree(&bindings); throw Win32Exception(RPC_S_NO_BINDINGS); }

		try {

			// If a non-nil object id was provided, initialize the type mapping
			if(!UuidIsNil(const_cast<uuid_t*>(&objectid), &rpcresult)) RpcObjectSetType(const_cast<uuid_t*>(&objectid), mgrtypeid);

			// Attempt to register the interface with the AUTOLISTEN flag specified
			rpcresult = RpcServerRegisterIfEx(*ifspec, mgrtypeid, mgrepv, RPC_IF_AUTOLISTEN, RPC_C_LISTEN_MAX_CALLS_DEFAULT, nullptr);
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			// Register the endpoint for the interface/type/object
			UUID_VECTOR objects = { 1, const_cast<uuid_t*>(&objectid) };
			rpcresult = RpcEpRegister(*ifspec, bindings, UuidIsNil(const_cast<uuid_t*>(&objectid), &rpcresult) ? nullptr : &objects, nullptr);
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			// The RpcAutoListener instance takes over the object id and the binding vector
			return std::make_unique<RpcAutoListener>(objectid, bindings);
		}

		catch(...) {

			// Clean up everything on exception and just re-throw it
			RpcServerUnregisterIfEx(*ifspec, mgrtypeid, TRUE);
			RpcBindingVectorFree(&bindings);
			if(!UuidIsNil(const_cast<uuid_t*>(&objectid), &rpcresult)) RpcObjectSetType(const_cast<uuid_t*>(&objectid), nullptr);

			throw;
		}
	}

	// Stop
	//
	// Stops and unregisters the RPC interface/endpoint
	void Stop(bool rundown = true)
	{
		RPC_STATUS			rpcresult;			// Result from RPC function call

		// Unregister the endpoint first, optionally sending in the object
		UUID_VECTOR objects = { 1, &m_objectid };
		rpcresult = RpcEpUnregister(*ifspec, m_bindings, UuidIsNil(&m_objectid, &rpcresult) ? nullptr : &objects);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

		// Unregister the interface, optionally waiting for calls to complete and context handle rundown
		rpcresult = RpcServerUnregisterIfEx(*ifspec, mgrtypeid, (rundown) ? TRUE : FALSE);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

		// If an object id was specified when starting the listener, reset the type mapping
		if(!UuidIsNil(&m_objectid, &rpcresult)) RpcObjectSetType(&m_objectid, nullptr);
	}

private:

	RpcAutoListener(const RpcAutoListener &rhs)=delete;
	RpcAutoListener& operator=(const RpcAutoListener &rhs)=delete;

	// Instance Constructor
	//
public:
	// TODO: FIX OR REMOVE ME
	RpcAutoListener(const uuid_t& objectid, RPC_BINDING_VECTOR* bindings) : m_objectid(objectid), m_bindings(bindings) {}
private:
	
	//-------------------------------------------------------------------------
	// Member Variables

	uuid_t					m_objectid;					// Object unique identifier
	RPC_BINDING_VECTOR*		m_bindings = nullptr;		// Binding vector
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCAUTOLISTENER_H_
