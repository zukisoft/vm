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
#include "VirtualMachine2.h"

#pragma warning(push, 4)

// VirtualMachine::s_objects
//
// Static collection of active VirtualMachine instances
VirtualMachine2::instance_map_t VirtualMachine2::s_instances;

// VirtualMachine::s_lock
//
// Synchronization object for concurrent access to the collection
VirtualMachine2::rwlock_t VirtualMachine2::s_lock;

//-----------------------------------------------------------------------------
// VirtualMachine::FindVirtualMachine (static)
//
// Retrieves a VirtualMachine instance from the static collection
//
// Arguments:
//
//	instanceid		- UUID to be mapped back to a VirtualMachine instance

std::shared_ptr<VirtualMachine2> VirtualMachine2::FindVirtualMachine(const uuid_t& instanceid)
{
	read_lock reader(s_lock);

	// Attempt to locate the instanceid in the collection
	auto iterator = s_instances.find(instanceid);
	return (iterator == s_instances.end()) ? nullptr : iterator->second->shared_from_this();
}

//-----------------------------------------------------------------------------
// VirtualMachine::RegisterInstance (static, protected)
//
// Registers a VirtualMachine instance to allow FindInstance() to locate it
//
// Arguments:
//
//	vm			- shared_ptr<> for the VirtualMachine instance to register
//	instanceid	- UUID that uniquely identifies the instance for FindInstance()

VirtualMachine2::VirtualMachine2()
{
	UuidCreate(&m_instanceid);			// Generate a unique identifier

	// Repeatedly try to insert the new instance, regenerating the UUID as necessary
	write_lock writer(s_lock);
	while(!s_instances.insert(std::make_pair(m_instanceid, this)).second) UuidCreate(&m_instanceid);
}

//-----------------------------------------------------------------------------
// VirtualMachine::UnregisterInstance (static, protected)
//
// Unregisters a VirtualMachine instance such that FindInstance() cannot see it.
//
// Arguments:
//
//	instanceid	- UUID that uniquely identifies the VirtualMachine instance

VirtualMachine2::~VirtualMachine2()
{
	write_lock writer(s_lock);
	s_instances.erase(m_instanceid);
}


//---------------------------------------------------------------------------

#pragma warning(pop)
