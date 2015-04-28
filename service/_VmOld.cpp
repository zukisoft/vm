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

#include "stdafx.h"
#include "_VmOld.h"

#pragma warning(push, 4)

// _VmOld::s_objects
//
// Static collection of active _VmOld instances
_VmOld::instance_map_t _VmOld::s_instances;

// _VmOld::s_lock
//
// Synchronization object for concurrent access to the collection
_VmOld::rwlock_t _VmOld::s_lock;

//-----------------------------------------------------------------------------
// _VmOld::CreateDeviceId (static)
//
// Creates a device identifier from major and minor components
//
// Arguments:
//
//	major		- Device major code
//	minor		- Device minor code

uapi::dev_t _VmOld::CreateDeviceId(uint32_t major, uint32_t minor)
{
	// MKDEV() from kdev_t.h
	return ((major << 20) | (minor));
}
	
//-----------------------------------------------------------------------------
// _VmOld::Find_VmOld (static)
//
// Retrieves a _VmOld instance from the static collection
//
// Arguments:
//
//	instanceid		- UUID to be mapped back to a _VmOld instance

std::shared_ptr<_VmOld> _VmOld::Find_VmOld(const uuid_t& instanceid)
{
	read_lock reader(s_lock);

	// Attempt to locate the instanceid in the collection and ask it to generate an
	// std::shared_ptr<_VmOld> from itself; understood this is a hack job
	auto iterator = s_instances.find(instanceid);
	return (iterator != s_instances.end()) ? iterator->second->ToSharedPointer() : nullptr;
}

//-----------------------------------------------------------------------------
// _VmOld Constructor (protected)
//
// Arguments:
//
//	NONE

_VmOld::_VmOld()
{
	UuidCreate(&m_instanceid);			// Generate a new unique identifier

	// Repeatedly try to insert the new instance, regenerating the UUID as necessary
	write_lock writer(s_lock);
	while(!s_instances.insert(std::make_pair(m_instanceid, this)).second) UuidCreate(&m_instanceid);
}

//-----------------------------------------------------------------------------
// _VmOld Destructor

_VmOld::~_VmOld()
{
	write_lock writer(s_lock);
	s_instances.erase(m_instanceid);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
