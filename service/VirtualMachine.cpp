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
#include "VirtualMachine.h"

#pragma warning(push, 4)

// VirtualMachine::s_instances
//
// Static collection of active virtual machine instances
VirtualMachine::instance_map_t VirtualMachine::s_instances;

// VirtualMachine::s_lock
//
// Synchronization object for active virtual machine collection
VirtualMachine::instance_map_lock_t VirtualMachine::s_instancelock;

//---------------------------------------------------------------------------
// VirtualMachine Constructor
//
// Arguments:
//
//	NONE

VirtualMachine::VirtualMachine() : m_instanceid(GenerateInstanceId())
{
}

//---------------------------------------------------------------------------
// VirtualMachine::Find (static)
//
// Locates an active virtual machine instance based on its uuid
//
// Arguments:
//
//	instanceid	- Virtual machine instance identifier

std::shared_ptr<VirtualMachine> VirtualMachine::Find(const uuid_t& instanceid)
{
	instance_map_lock_t::scoped_lock_read reader(s_instancelock);

	// Attempt to locate the instanceid in the collection
	auto iterator = s_instances.find(instanceid);
	return (iterator != s_instances.end()) ? iterator->second : nullptr;
}

//---------------------------------------------------------------------------
// VirtualMachine::GenerateInstanceId (static, private)
//
// Generate the universally unique identifier for this virtual machine instance
//
// Arguments:
//
//	NONE

uuid_t VirtualMachine::GenerateInstanceId(void)
{
	uuid_t			instanceid;			// Generated instance identifier

	// Attempt to generate a new uuid_t for this virtual machine instance
	RPC_STATUS rpcresult = UuidCreate(&instanceid);
	if(rpcresult != RPC_S_OK) throw Win32Exception(rpcresult);

	return instanceid;
}

//---------------------------------------------------------------------------
// VirtualMachine::OnStart (private)
//
// Invoked when the service is started
//
// Arguments:
//
//	argc		- Number of command line arguments
//	argv		- Array of command line argument strings

void VirtualMachine::OnStart(int argc, LPTSTR* argv)
{
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	try {

		// Construct the root namespace for this virtual machine instance
		m_rootns = Namespace::Create();

		// Add this virtual machine instance to the active instance collection
		instance_map_lock_t::scoped_lock writer(s_instancelock);
		s_instances.emplace(m_instanceid, shared_from_this());
	}

	// Win32Exception and Exception can be translated into a ServiceException
	catch(Win32Exception& ex) { throw ServiceException(static_cast<DWORD>(ex.Code)); }
	catch(Exception& ex) { throw ServiceException(ex.HResult); }
}

//---------------------------------------------------------------------------
// VirtualMachine::OnStop
//
// Invoked when the service is stopped
//
// Arguments:
//
//	NONE

void VirtualMachine::OnStop(void)
{
	// Remove this virtual machine from the active instance collection
	instance_map_lock_t::scoped_lock writer(s_instancelock);
	s_instances.erase(m_instanceid);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
