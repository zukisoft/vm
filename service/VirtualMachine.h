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

#ifndef __VIRTUALMACHINE_H_
#define __VIRTUALMACHINE_H_
#pragma once

#include <memory>
#include <tuple>
#include <unordered_map>
#include "Architecture.h"
#include "Context.h"
#include "FileSystem.h"

// Forward Declarations
//
class Namespace;
class NativeProcess;
class NativeThread;
class Pid;
class Process;
class RpcObject;
class Session;

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Class VirtualMachine
//
// Implements the top-level virtual machine instance

class VirtualMachine : public Service<VirtualMachine>, public std::enable_shared_from_this<VirtualMachine>
{
public:

	// Instance Constructor
	//
	VirtualMachine();

	// Destructor
	//
	~VirtualMachine()=default;

	//-------------------------------------------------------------------------
	// Friend Functions

	// AddVirtualMachineSession
	//
	// Adds a session to the collection
	friend std::shared_ptr<VirtualMachine> AddVirtualMachineSession(std::shared_ptr<VirtualMachine> vm, std::shared_ptr<Session> session);

	// RemoveVirtualMachineSession
	//
	// Removes a session from the collection
	friend void RemoveVirtualMachineSession(std::shared_ptr<VirtualMachine> vm, Session const* session);

	//-------------------------------------------------------------------------
	// Member Functions

	// CreateHost
	//
	// Creates a new Host instance for the specified architecture
	std::tuple<std::unique_ptr<NativeProcess>, std::unique_ptr<NativeThread>> CreateHost(enum class Architecture architecture);

	// Find (static)
	//
	// Locates a virtual machine instance based on its uuid
	static std::shared_ptr<VirtualMachine> Find(uuid_t const& instanceid);

	//-------------------------------------------------------------------------
	// Properties

	// InstanceId
	//
	// Gets the unique identifier for this virtual machine instance
	__declspec(property(get=getInstanceId)) uuid_t InstanceId;
	uuid_t getInstanceID(void) const { return m_instanceid; }

private:

	VirtualMachine(VirtualMachine const&)=delete;
	VirtualMachine& operator=(VirtualMachine const&)=delete;

	// VirtualMachine::Context
	//
	// Implements an RAII context implementation for the virtual machine
	class Context : public ::Context
	{
	public:

		// Instance Constructor
		//
		Context();

		// Destructor
		//
		virtual ~Context();
	};

	// VirtualMachine::RootAlias
	//
	// Implements the absolute root of the virtual file system (/)
	class RootAlias : public FileSystem::Alias
	{
	public:

		// Instance Constructor
		//
		RootAlias(std::shared_ptr<FileSystem::Directory> dir);

		// Destructor
		//
		~RootAlias()=default;

		//---------------------------------------------------------------------
		// FileSystem::Alias Implementation

		// GetName
		//
		// Reads the name assigned to this alias
		virtual uapi::size_t GetName(char_t* buffer, size_t count) const;

		// Name
		//
		// Gets the name assigned to this alias
		virtual std::string getName(void) const;

		// Node
		//
		// Gets the node to which this alias refers
		virtual std::shared_ptr<FileSystem::Node> getNode(void) const;

	private:

		RootAlias(RootAlias const&)=delete;
		RootAlias& operator=(RootAlias const&)=delete;

		//-------------------------------------------------------------------------
		// Member Variables

		std::shared_ptr<FileSystem::Directory> const	m_dir;	// Attached directory
	};

	// Service<> Control Handler Map
	//
	BEGIN_CONTROL_HANDLER_MAP(VirtualMachine)
		CONTROL_HANDLER_ENTRY(SERVICE_CONTROL_STOP, OnStop)
	END_CONTROL_HANDLER_MAP()

	// Service<> Parameter Map
	//
	BEGIN_PARAMETER_MAP(VirtualMachine)
		PARAMETER_ENTRY(_T("init"),				m_paraminit)				// String
		PARAMETER_ENTRY(_T("initrd"),			m_paraminitrd)				// String
		PARAMETER_ENTRY(_T("ro"),				m_paramro)					// DWord
		PARAMETER_ENTRY(_T("root"),				m_paramroot)				// String
		PARAMETER_ENTRY(_T("rootfstype"),		m_paramrootfstype)			// String
		PARAMETER_ENTRY(_T("rootflags"),		m_paramrootflags)			// String
		PARAMETER_ENTRY(_T("rw"),				m_paramrw)					// DWord

		// todo: dots in the argument name implies that it's a module parameter, which
		// is a good thing, but decide on the proper name for it -- "vm" is a tad generic
		PARAMETER_ENTRY(_T("vm.host32"),		m_paramhost32)				// String
		PARAMETER_ENTRY(_T("vm.host64"),		m_paramhost64)				// String

	END_PARAMETER_MAP()

	// filesystem_map_t
	//
	// Collection of available file systems (name, mount function)
	using filesystem_map_t = std::unordered_map<std::string, FileSystem::MountFunction>;

	// fsmount_t
	//
	// FileSystem::Mount shared pointer
	using fsmount_t = std::shared_ptr<FileSystem::Mount>;

	// fspath_t
	//
	// FileSystem::Path shared pointer
	using fspath_t = std::shared_ptr<FileSystem::Path>;

	// fsprocess_t
	//
	// Process shared pointer
	using fsprocess_t = std::shared_ptr<Process>;

	// instance_map_t
	//
	// Collection of virtual machine instances
	using instance_map_t = std::unordered_map<uuid_t, std::shared_ptr<VirtualMachine>>;

	// session_map_t
	//
	// Collection of session instances
	using session_map_t = std::unordered_map<Session const*, std::weak_ptr<Session>>;

	// GenerateInstanceId (static)
	//
	// Generates the universally unique identifier for this instance
	static uuid_t GenerateInstanceId(void);

	// OnStart (Service)
	//
	// Invoked when the service is started
	virtual void OnStart(int argc, LPTSTR* argv);

	// OnStop
	//
	// Invoked when the service is stopped
	void OnStop(void);

	//-------------------------------------------------------------------------
	// Member Variables

	static instance_map_t			s_instances;		// Collection of all instances
	static sync::reader_writer_lock	s_instancelock;		// Synchronization object

	uuid_t const					m_instanceid;		// Instance identifier
	std::unique_ptr<RpcObject>		m_syscalls32;		// 32-bit system calls object
	std::unique_ptr<RpcObject>		m_syscalls64;		// 64-bit system calls object

	std::shared_ptr<Namespace>		m_rootns;			// Root namespace instance

	// Job
	//
	HANDLE							m_job;				// Job object for all processes

	// Sessions
	//
	session_map_t					m_sessions;			// Collection of Sessions
	mutable sync::critical_section	m_sessionslock;		// Synchronization object

	// File Systems
	//
	filesystem_map_t				m_filesystems;		// Available file systems
	fsmount_t						m_rootmount;		// Root file system mount
	fspath_t						m_rootpath;			// Path to the root node

	// Init Process
	fsprocess_t						m_initprocess;		// Initial process

	// Service<> Parameters
	//
	StringParameter					m_paraminit			{ _T("/sbin/init") };
	StringParameter					m_paraminitrd;
	DWordParameter					m_paramro			{ 1 };
	StringParameter					m_paramroot;
	StringParameter					m_paramrootfstype	{ _T("rootfs") };
	StringParameter					m_paramrootflags;
	DWordParameter					m_paramrw			{ 0 };
	StringParameter					m_paramhost32;
	StringParameter					m_paramhost64;
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALMACHINE_H_
