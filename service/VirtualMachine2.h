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

#ifndef __VIRTUALMACHINE2_H_		// <--- TODO: REMOVE "2"
#define __VIRTUALMACHINE2_H_
#pragma once

#include <concrt.h>
#include <map>
#include <memory>
#include <linux/types.h>

#pragma warning(push, 4)

// Forward Declarations
//
class Process;

//-----------------------------------------------------------------------------
// Class VirtualMachine
//
// VirtualMachine provides the abstraction layer that sits between the system
// calls interface and the underlying implementation.  Only functions that are
// needed by system calls should be defined in this class, and they should use
// exceptions to indicate failure conditions -- don't return linux error codes, 
// throw a LinuxException.

class VirtualMachine2 : private std::enable_shared_from_this<VirtualMachine2>
{
public:

	// Destructor
	//
	~VirtualMachine2();

	//-------------------------------------------------------------------------
	// Member Functions

	// FindVirtualMachine (static)
	//
	// Locates an active VirtualMachine instance and returns its shared_ptr<>
	static std::shared_ptr<VirtualMachine2> FindVirtualMachine(const uuid_t& instanceid);

	//-------------------------------------------------------------------------
	// Virtual Machine Interface

	//
	// TODO: all the functions needed by the system calls layer go here -- will
	// need to start reworking that to know how this will pan out
	//
	
	// EXAMPLE OF BAD API: SYSTEM CALLS NEVER DO THIS -- THEY FORK/CLONE:
	//virtual std::shared_ptr<Process> CreateProcess(const uapi::char_t* path, const uapi::char_t** arguments, const uapi::char_t** environment) = 0;

	// EXAMPLE OF GOOD API: SYSTEM CALLS DO THIS:
	//virtual FileSystem::HandlePtr CreateFile(const uapi::char_t* path);

	// FindProcess
	//
	// Locates and retrieves a Process instance based on a hosted process PID
	//virtual std::shared_ptr<Process> FindProcess(DWORD hostpid) = 0;

	// FindProcess
	// KillProcess
	// KillProcessGroup

	// OpenFile
	// CreateFile
	// CreateDirectory
	// CreateSymbolicLink

	// DomainName
	//
	// Gets/sets the current domain name for the virtual machine
	__declspec(property(get=getDomainName, put=putDomainName)) const uapi::char_t* DomainName;
	virtual const uapi::char_t* getDomainName(void) = 0;
	virtual void putDomainName(const uapi::char_t* value) = 0;

	// HarwareIdentifier
	//
	// Gets the virtual machine machine identifier, for example "x86_64"
	__declspec(property(get=getHardwareIdentifier)) const uapi::char_t* HardwareIdentifier;
	virtual const uapi::char_t* getHardwareIdentifier(void) = 0;

	// HostName
	//
	// Gets/sets the current host name for the virtual machine
	__declspec(property(get=getHostName, put=putHostName)) const uapi::char_t* HostName;
	virtual const uapi::char_t* getHostName(void) = 0;
	virtual void putHostName(const uapi::char_t* value) = 0;

	// OperatingSystemRelease
	//
	// Gets the operating system release, for example "3.13.0.37"
	__declspec(property(get=getOperatingSystemRelease)) const uapi::char_t* OperatingSystemRelease;
	virtual const uapi::char_t* getOperatingSystemRelease(void) = 0;

	// OperatingSystemType
	//
	// Gets the operating system type, for example "Linux"
	__declspec(property(get=getOperatingSystemType)) const uapi::char_t* OperatingSystemType;
	virtual const uapi::char_t* getOperatingSystemType(void) = 0;

	// Version
	//
	// Gets the virtual machine version, this is for the most part a free-form string
	__declspec(property(get=getVersion)) const uapi::char_t* Version;
	virtual const uapi::char_t* getVersion(void) = 0;

protected:

	// Instance Constructor
	//
	VirtualMachine2();

private:

	VirtualMachine2(const VirtualMachine2&)=delete;
	VirtualMachine2& operator=(const VirtualMachine2&)=delete;

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// uuid_key_comp_t
	//
	// Key comparison type for UUIDs when used as a collection key
	struct uuid_key_comp_t 
	{ 
		bool operator() (const uuid_t& lhs, const uuid_t& rhs) const { return (memcmp(&lhs, &rhs, sizeof(uuid_t)) < 0); }
	};

	// instance_map_t
	//
	// Instance ID -> VirtualMachine implementation collection
	using instance_map_t = std::map<uuid_t, VirtualMachine2*, uuid_key_comp_t>;

	// rw_lock_t, write_lock, read_lock
	//
	// Mappings for Concurrency::reader_writer_lock
	using rwlock_t = Concurrency::reader_writer_lock;
	using write_lock = Concurrency::reader_writer_lock::scoped_lock;
	using read_lock = Concurrency::reader_writer_lock::scoped_lock_read;

	//-------------------------------------------------------------------------
	// Member Variables

	uuid_t					m_instanceid;	// VirtualMachine instance identifier
	static instance_map_t	s_instances;	// Object instance collection
	static rwlock_t			s_lock;			// Collection synchronization lock
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALMACHINE_H_
