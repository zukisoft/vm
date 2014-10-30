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

#ifndef __VIRTUALMACHINE_H_
#define __VIRTUALMACHINE_H_
#pragma once

#include <concrt.h>
#include <map>
#include <memory>
#include <linux/types.h>
#include "FileSystem.h"

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
// throw a LinuxException
//
// The final class that inherits from VirtualMachine needs to derived from the
// std::enable_shared_from_this<> class and provide an implementation of the
// virtual GetSharedPointer() method.  The original plan was to have this class
// derived from enable_shared_from_this<> so it could implement the instance
// find method on its own, but that didn't really work out

//////  THIS SHOULD BE AN INTERFACE ONLY, LIKE FILESYSTEM
//////  ALSO SHOULD HAVE VirtualMachine::Process THESE ARE THE OBJECTS THAT THE
//////  SYSTEM CALLS INTERACT WITH

class VirtualMachine
{
public:

	// Destructor
	//
	~VirtualMachine();

	// move me?
	enum class Properties
	{
		DomainName					= 0,
		HardwareIdentifier,
		HostName,
		OperatingSystemRelease,
		OperatingSystemType,
		OperatingSystemVersion,
	};

	//-------------------------------------------------------------------------
	// Member Functions

	// FindProcessByHostID	// <--- TODO: Rename/repurpose to allow a wait for the host to register?
	//
	// Locates a Process instance by the host process' PID, necessary for the system
	// call interface to establish the context
	virtual std::shared_ptr<Process> FindProcessByHostID(uint32_t hostpid) = 0;

	// FindVirtualMachine (static)
	//
	// Locates an active VirtualMachine instance and returns its shared_ptr<>
	static std::shared_ptr<VirtualMachine> FindVirtualMachine(const uuid_t& instanceid);

	// OpenExecutable
	//
	// Opens a file system object for execute-only access
	virtual std::shared_ptr<FileSystem::Handle> OpenExecutable(const uapi::char_t* path) = 0;

	virtual std::shared_ptr<FileSystem::Handle> OpenFile(const uapi::char_t* pathname, int flags, uapi::mode_t mode) = 0;

	//
	// PROPERTY MANAGEMENT
	//

	// GetProperty (std::string)
	//
	// Retrieves a property as a std::string instance
	virtual std::string GetProperty(VirtualMachine::Properties id) = 0;

	// GetProperty (char_t*)
	//
	// Retrieves a property by writing it into a character buffer
	virtual size_t GetProperty(VirtualMachine::Properties id, uapi::char_t* value, size_t length) = 0;

	// SetProperty (std::string)
	//
	// Sets the value of a property from an std::string
	virtual void SetProperty(VirtualMachine::Properties id, std::string value) = 0;

	// SetProperty (char_t*)
	//
	// Sets the value of a property from a null-terminated character buffer
	virtual void SetProperty(VirtualMachine::Properties id, const uapi::char_t* value) = 0;

	// SetProperty (char_t*)
	//
	// Sets the value of a property from a fixed-length character buffer
	virtual void SetProperty(VirtualMachine::Properties id, const uapi::char_t* value, size_t length) = 0;

	//-------------------------------------------------------------------------
	// Properties

	// InstanceID
	//
	// Gets the unique identifier for this virtual machine instance
	__declspec(property(get=getInstanceID)) const uuid_t& InstanceID;
	const uuid_t& getInstanceID(void) const { return m_instanceid; }

protected:

	// Instance Constructor
	//
	VirtualMachine();

	// ToSharedPointer
	//
	// Gets an std::shared_ptr<VirtualMachine> from the derived class
	virtual std::shared_ptr<VirtualMachine> ToSharedPointer(void) = 0;

private:

	VirtualMachine(const VirtualMachine&)=delete;
	VirtualMachine& operator=(const VirtualMachine&)=delete;

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
	using instance_map_t = std::map<uuid_t, VirtualMachine*, uuid_key_comp_t>;

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
