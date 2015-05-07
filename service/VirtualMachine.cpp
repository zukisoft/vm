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

// System Call RPC Interfaces
//
#include <syscalls32.h>
#ifdef _M_X64
#include <syscalls64.h>
#endif

// File Systems
//
#include "HostFileSystem.h"
#include "RootFileSystem.h"
#include "TempFileSystem.h"

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
	// The command line arguments should be used to override defaults
	// set in the service parameters.  This functionality should be
	// provided by servicelib directly -- look into that
	UNREFERENCED_PARAMETER(argc);
	UNREFERENCED_PARAMETER(argv);

	try {

		// ROOT NAMESPACE
		//
		m_rootns = Namespace::Create();

		// PROPERTIES
		//

		// SYSTEM LOG
		//

		// FILE SYSTEMS
		//
		m_filesystems.emplace("hostfs",	HostFileSystem::Mount);
		m_filesystems.emplace("rootfs",	RootFileSystem::Mount);
		m_filesystems.emplace("tmpfs",	TempFileSystem::Mount);

		// ROOT FILE SYSTEM
		//
		auto root = std::to_string(m_paramroot.Value);
		auto rootflags = std::to_string(m_paramrootflags.Value);
		auto rootfstype = std::to_string(m_paramrootfstype.Value);

		try { m_rootfs = m_filesystems.at(rootfstype.c_str())(root.c_str(), MountOptions::Parse(LINUX_MS_KERNMOUNT, rootflags.c_str())); }
		catch(...) { throw; } // <--- todo

		// INITRAMFS
		//
		auto initrd = std::to_string(m_paraminitrd.Value);

		// INIT PROCESS
		//
		auto init = std::to_string(m_paraminit.Value);

		auto initpid = m_rootns->Pid->CreatePid();
		_ASSERTE(initpid->Value[m_rootns] == 1);

		// SYSTEM CALL RPC OBJECTS
		//
		m_syscalls32 = RpcObject::Create(SystemCalls32_v1_0_s_ifspec, m_instanceid, RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY);
#ifdef _M_X64
		m_syscalls64 = RpcObject::Create(SystemCalls64_v1_0_s_ifspec, m_instanceid, RPC_IF_AUTOLISTEN | RPC_IF_ALLOW_SECURE_ONLY);
#endif

		// NOTE: DON'T START INIT PROCESS UNTIL AFTER THE RPC OBJECTS HAVE BEEN CONSTRUCTED
	}

	// Win32Exception and Exception can be translated into ServiceExceptions
	catch(Win32Exception& ex) { throw ServiceException(static_cast<DWORD>(ex.Code)); }
	catch(Exception& ex) { throw ServiceException(ex.HResult); }
	// catch(std::exception& ex) { /* PUT SOMETHING HERE */ }

	// Add this virtual machine instance to the active instance collection
	instance_map_lock_t::scoped_lock writer(s_instancelock);
	s_instances.emplace(m_instanceid, shared_from_this());
}

//---------------------------------------------------------------------------
// VirtualMachine::OnStop (private)
//
// Invoked when the service is stopped
//
// Arguments:
//
//	NONE

void VirtualMachine::OnStop(void)
{
	m_syscalls32.reset();			// Revoke the 32-bit system calls object
#ifdef _M_X64
	m_syscalls64.reset();			// Revoke the 64-bit system calls object
#endif

	// Remove this virtual machine from the active instance collection
	instance_map_lock_t::scoped_lock writer(s_instancelock);
	s_instances.erase(m_instanceid);
}

//---------------------------------------------------------------------------

#pragma warning(pop)
