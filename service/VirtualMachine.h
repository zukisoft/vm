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

#include <deque>
#include <map>
#include <memory>
#include <stack>
#include <linux/fs.h>
#include "Exception.h"
#include "FileSystem.h"
#include "MountOptions.h"
#include "Namespace.h"
#include "RpcObject.h"
#include "Win32Exception.h"

#pragma warning(push, 4)

//-----------------------------------------------------------------------------
// Class VirtualMachine
//
// Implements the top-level virtual machine instance

class VirtualMachine : public Service<VirtualMachine>, public std::enable_shared_from_this<VirtualMachine>
{
public:

	// Constructor / Destructor
	//
	VirtualMachine();
	~VirtualMachine()=default;

	//-------------------------------------------------------------------------
	// Member Functions

	// Find (static)
	//
	// Locates a virtual machine instance based on its uuid
	static std::shared_ptr<VirtualMachine> Find(const uuid_t& instanceid);

	//-------------------------------------------------------------------------
	// Properties

	// InstanceId
	//
	// Gets the unique identifier for this virtual machine instance
	__declspec(property(get=getInstanceId)) uuid_t InstanceId;
	uuid_t getInstanceID(void) const { return m_instanceid; }

private:

	VirtualMachine(const VirtualMachine&)=delete;
	VirtualMachine& operator=(const VirtualMachine&)=delete;

	// VirtualMachine::RootAlias
	//
	// Implements the absolute root of the virtual file system (/)
	class RootAlias : public FileSystem::Alias
	{
	public:

		// Instance Constructor
		//
		RootAlias(std::shared_ptr<FileSystem::Node> node);

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

		RootAlias(const RootAlias&)=delete;
		RootAlias& operator=(const RootAlias&)=delete;

		//-------------------------------------------------------------------------
		// Member Variables

		const std::shared_ptr<FileSystem::Node>	m_node;		// Attached node
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
	END_PARAMETER_MAP()

	// uuid_key_comp_t
	//
	// Comparison type for uuid_t when used as a collection key
	struct uuid_key_comp_t 
	{ 
		bool operator() (const uuid_t& lhs, const uuid_t& rhs) const { return (memcmp(&lhs, &rhs, sizeof(uuid_t)) < 0); }
	};

	// filesystem_map_t
	//
	// Collection of available file systems (name, mount function)
	using filesystem_map_t = std::map<std::string, FileSystem::MountFunction>;

	// instance_map_t
	//
	// Collection of virtual machine instances
	using instance_map_t = std::map<uuid_t, std::shared_ptr<VirtualMachine>, uuid_key_comp_t>;

	// instance_map_lock_t
	//
	// Synchronization object for the instance collection
	using instance_map_lock_t = sync::reader_writer_lock;

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

	static instance_map_t		s_instances;		// Collection of all instances
	static instance_map_lock_t	s_instancelock;		// Synchronization object

	const uuid_t				m_instanceid;		// Instance identifier
	std::unique_ptr<RpcObject>	m_syscalls32;		// 32-bit system calls object

#ifdef _M_X64
	std::unique_ptr<RpcObject>	m_syscalls64;		// 64-bit system calls object
#endif

	std::shared_ptr<Namespace>	m_rootns;			// Root namespace instance

	// File Systems
	//
	filesystem_map_t					m_filesystems;		// Available file systems
	std::shared_ptr<FileSystem::Alias>	m_rootalias;		// Root file system alias (/)

	// Service<> Parameters
	//
	StringParameter				m_paraminit			{ _T("/sbin/init") };
	StringParameter				m_paraminitrd;
	DWordParameter				m_paramro			{ 1 };
	StringParameter				m_paramroot;
	StringParameter				m_paramrootfstype	{ _T("rootfs") };
	StringParameter				m_paramrootflags;
	DWordParameter				m_paramrw			{ 0 };
};

//-----------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VIRTUALMACHINE_H_
