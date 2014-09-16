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

#include "stdafx.h"
#include "SystemCalls.h"

#pragma warning(push, 4)

// SystemCalls::s_objects
//
// Static collection of active SystemCall instances
SystemCalls::object_map_t SystemCalls::s_objects;

// SystemCalls::s_lock
//
// Synchronization object for concurrent access to the collection
SystemCalls::rwlock_t SystemCalls::s_lock;

//-----------------------------------------------------------------------------
// SystemCalls Constructor (protected)
//
// Arguments:
//
//	NONE

SystemCalls::SystemCalls()
{
	// Generate unique identifiers that can be passed into the RPC runtime
	UuidCreate(&m_objectid32);
	UuidCreate(&m_objectid64);

	// Repeatedly try to insert the new objects, regenerating the UUID as necessary
	write_lock writer(s_lock);
	while(!s_objects.insert(std::make_pair(m_objectid32, this)).second) UuidCreate(&m_objectid32);
	while(!s_objects.insert(std::make_pair(m_objectid64, this)).second) UuidCreate(&m_objectid64);
}

//-----------------------------------------------------------------------------
// SystemCalls Destructor

SystemCalls::~SystemCalls()
{
	write_lock writer(s_lock);
	s_objects.erase(m_objectid32);
	s_objects.erase(m_objectid64);
}

//-----------------------------------------------------------------------------
// SystemCalls::FromObjectID (static)
//
// Retrieves a SystemCalls instance from the static collection
//
// Arguments:
//
//	objectid		- ObjectID to be mapped back to SystemCalls*

SystemCalls* SystemCalls::FromObjectID(const uuid_t& objectid)
{
	read_lock reader(s_lock);

	// Attempt to locate the objectid in the collection and return null
	// if it doesn't map to an active SystemCalls instance
	auto iterator = s_objects.find(objectid);
	return (iterator == s_objects.end()) ? nullptr : iterator->second;
}

//---------------------------------------------------------------------------

#pragma warning(pop)
