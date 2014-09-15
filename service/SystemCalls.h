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

#ifndef __SYSTEMCALLS_H_
#define __SYSTEMCALLS_H_
#pragma once

#include <concrt.h>
#include <map>

#pragma warning(push, 4)			

//-----------------------------------------------------------------------------
// Class SystemCalls (abstract)
//
// Generic implementation of the server system calls interface.  When constructed,
// a unique identifier is generated and the instance is stored in a static collection
// that the RPC entry point vectors can use to thunk from the static entry points
// based on just the client binding handle

class SystemCalls
{
public:

	// Destructor
	//
	virtual ~SystemCalls();

	//-------------------------------------------------------------------------
	// Member Functions

	// FromObjectID
	//
	// Retrieves a SystemCalls instance based on it's object id
	static SystemCalls* FromObjectID(const uuid_t& objectid);

	//
	// TODO: the API goes here
	//

protected:

	// Instance Constructor
	//
	SystemCalls();

	// ObjectID
	//
	// Exposes the auto-generated instance identifier
	__declspec(property(get=getObjectID)) uuid_t ObjectID;
	uuid_t getObjectID(void) const { return m_objectid; }

private:

	SystemCalls(const SystemCalls&)=delete;
	SystemCalls& operator=(const SystemCalls&)=delete;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// uuid_key_comp_t
	//
	// Key comparison type for UUIDs when used as a collection key
	struct uuid_key_comp_t 
	{ 
		bool operator() (const uuid_t& lhs, const uuid_t& rhs) const { return (memcmp(&lhs, &rhs, sizeof(uuid_t)) < 0); }
	};

	// object_map_t
	//
	// RPC object -> SystemCalls interface map type
	using object_map_t = std::map<uuid_t, SystemCalls*, uuid_key_comp_t>;

	// rw_lock_t, write_lock, read_lock
	//
	// Mappings for Concurrency::reader_writer_lock
	using rwlock_t = Concurrency::reader_writer_lock;
	using write_lock = Concurrency::reader_writer_lock::scoped_lock;
	using read_lock = Concurrency::reader_writer_lock::scoped_lock_read;

	//-------------------------------------------------------------------------
	// Member Variables

	uuid_t						m_objectid;		// Instance identifier

	static object_map_t			s_objects;		// Object instance collection
	static rwlock_t				s_lock;			// Collection synchronization lock
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __SYSTEMCALLS_H_
