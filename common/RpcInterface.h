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

#ifndef __RPCINTERFACE_H_
#define __RPCINTERFACE_H_
#pragma once

#include "generic_text.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// TODO: I'm not very happy with this, but it will do for now.  Ideally I would
// prefer to have a nice smart implementation in servicelib, but even doing it
// standalone like this has a lot more to concern itself with than what's done here.
//
// This will work OK for now -- does the job
//
// RpcInterface<blah>::Register
// RpcInterface<blah>::AddObject
// ... do stuff ...
// RpcInterface<blah>::RemoveObject
// RpcInterface<blah>::Unregister

template <RPC_IF_HANDLE* ifspec, uuid_t* mgrtypeid = nullptr, RPC_MGR_EPV* mgrepv = nullptr>
class RpcInterface
{
public:

	//-------------------------------------------------------------------------
	// Member Functions

	// AddObject (static)
	//
	// Adds an object -> type mapping to resolve to this interface
	static void AddObject(const uuid_t& objectid)
	{
		RPC_BINDING_VECTOR*			bindings;			// RPC binding handles
		RPC_STATUS					rpcresult;			// Result from RPC function call

		// The server must have available protocol sequence bindings
		rpcresult = RpcServerInqBindings(&bindings);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

		try {
		
			// If there are no bindings, the object endpoint cannot be registered
			if(bindings->Count == 0) throw Win32Exception(RPC_S_NO_BINDINGS);

			// Associate the object id with this interface's manager type uuid
			rpcresult = RpcObjectSetType(const_cast<uuid_t*>(&objectid), mgrtypeid);
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			// Add an endpoint for the object
			UUID_VECTOR objects = { 1, const_cast<uuid_t*>(&objectid) };
			rpcresult = RpcEpRegister(*ifspec, bindings, &objects, nullptr);
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			RpcBindingVectorFree(&bindings);			// Release bindings vector
		}

		catch(...) { 
			
			RpcObjectSetType(const_cast<uuid_t*>(&objectid), nullptr);
			RpcBindingVectorFree(&bindings); 
			throw; 
		}
	}

	// GetBindingString (static)
	//
	// Hackish way to get a binding string - does not take anything into account
	// other than the first server binding - see general commentary above about
	// how I don't like this class as-is, but it will do for now
	static std::tstring GetBindingString(const uuid_t& objectid)
	{
		RPC_BINDING_VECTOR*			bindings;				// Server bindings
		RPC_BINDING_HANDLE			copy;					// Copy of server binding 0
		rpc_tchar_t*				strbinding;				// String binding
		std::tstring				result;					// Result from this function
		RPC_STATUS					rpcresult;				// Result from RPC function call

		rpcresult = RpcServerInqBindings(&bindings);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

		try {

			// If there are no bindings, there are no endpoints to retrieve
			if(bindings->Count == 0) throw Win32Exception(RPC_S_NO_BINDINGS);

			// Create a copy of the first binding handle in the vector
			rpcresult = RpcBindingCopy(bindings->BindingH[0], &copy);
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			// Associate the object id with the binding
			rpcresult = RpcBindingSetObject(copy, const_cast<uuid_t*>(&objectid));
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			// Convert the binding into a string binding
			rpcresult = RpcBindingToStringBinding(copy, &strbinding);
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			// Convert the string binding into an std::tstring and return it
			result = std::tstring(reinterpret_cast<tchar_t*>(strbinding));
			RpcStringFree(&strbinding);
			return result;
		}

		catch(...) { RpcBindingVectorFree(&bindings); throw; }
	}

	// Register (static)
	//
	// Registers the RPC interface
	static void Register(unsigned int flags)
	{
		// Attempt to register the interface with the specified flags
		RPC_STATUS rpcresult = RpcServerRegisterIfEx(*ifspec, mgrtypeid, mgrepv, flags, RPC_C_LISTEN_MAX_CALLS_DEFAULT, nullptr);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);
	}

	// RemoveObject (static)
	//
	// Removes an object -> type mapping
	static void RemoveObject(const uuid_t& objectid)
	{
		RPC_BINDING_VECTOR*			bindings;			// RPC binding handles
		RPC_STATUS					rpcresult;			// Result from RPC function call

		// The server must have available protocol sequence bindings
		rpcresult = RpcServerInqBindings(&bindings);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

		try {
		
			// If there are no bindings, the object endpoint cannot be unregistered
			if(bindings->Count == 0) throw Win32Exception(RPC_S_NO_BINDINGS);

			UUID_VECTOR objects = { 1, const_cast<uuid_t*>(&objectid) };
			rpcresult = RpcEpUnregister(*ifspec, bindings, &objects);
			if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

			// Disassociate the object id and release the binding vector
			RpcObjectSetType(const_cast<uuid_t*>(&objectid), nullptr);
			RpcBindingVectorFree(&bindings);
		}

		catch(...) { RpcBindingVectorFree(&bindings); throw; }
	}

	// Unregister (static)
	//
	// Unregisters the RPC interface
	static void Unregister(bool rundown = true)
	{
		// Unregister the interface, optionally waiting for calls to complete and context handle rundown
		RPC_STATUS rpcresult = RpcServerUnregisterIfEx(*ifspec, mgrtypeid, (rundown) ? TRUE : FALSE);
		if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);
	}

private:

	RpcInterface(const RpcInterface &rhs)=delete;
	RpcInterface& operator=(const RpcInterface &rhs)=delete;

	// Constructor / Destructor
	//
	RpcInterface()=default;
	virtual ~RpcInterface()=default;
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __RPCINTERFACE_H_
