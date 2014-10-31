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

#ifndef __VMSERVICE_H_
#define __VMSERVICE_H_
#pragma once

#include <map>
#include "resource.h"
#include "CompressedStreamReader.h"
#include "CpioArchive.h"
#include "Exception.h"
#include "File.h"
#include "FileSystem.h"
#include "RpcInterface.h"
#include "VirtualMachine.h"
#include "VmFileSystem.h"
#include "VmProcessManager.h"
#include "VmSystemLog.h"

#include "RootFileSystem.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// Class VmService
//
// TODO: oh so many words need to go here

// Needs to inherit from enable_shared_from_this<VmService> to trigger the 
// use of std::shared_ptr<> in servicelib.  Shared pointer currently needs to
// be used to implement VirtualMachine

class VmService : public Service<VmService>, public VirtualMachine,	public std::enable_shared_from_this<VmService>
{
public:

	VmService()=default;

	// VirtualMachine Implementation
	//
	virtual std::shared_ptr<Process>			FindProcessByHostID(uint32_t hostpid);
	virtual std::shared_ptr<FileSystem::Handle>	OpenExecutable(const uapi::char_t* path);
	virtual std::shared_ptr<FileSystem::Handle> OpenFile(const uapi::char_t* pathname, int flags, uapi::mode_t mode);

	virtual std::string		GetProperty(VirtualMachine::Properties id);
	virtual size_t			GetProperty(VirtualMachine::Properties id, uapi::char_t* value, size_t length);
	virtual void			SetProperty(VirtualMachine::Properties id, std::string value);
	virtual void			SetProperty(VirtualMachine::Properties id, const uapi::char_t* value);
	virtual void			SetProperty(VirtualMachine::Properties id, const uapi::char_t* value, size_t length);

private:

	VmService(const VmService &rhs)=delete;
	VmService& operator=(const VmService &rhs)=delete;

	// TEST - HACK JOB TO SOLVE THE PROBLEM FOR NOW
	// GET RID OF THIS
	virtual std::shared_ptr<VirtualMachine> ToSharedPointer(void)
	{
		return shared_from_this();
	}

	// Service<> Control Handler Map
	//
	BEGIN_CONTROL_HANDLER_MAP(VmService)
		CONTROL_HANDLER_ENTRY(SERVICE_CONTROL_STOP, OnStop)
	END_CONTROL_HANDLER_MAP()

	// LoadInitialFileSystem
	//
	// Loads the initial file system from an initramfs CPIO archive
	// THIS MOVES TO THE VFS; WHY IS IT HERE
	void LoadInitialFileSystem(const tchar_t* archivefile);

	// OnStart (Service)
	//
	// Invoked when the service is started
	void OnStart(int argc, LPTSTR* argv);

	// OnStop
	//
	// Invoked when the service is stopped
	void OnStop(void);

	//-------------------------------------------------------------------------
	// Private Type Declarations

	// property_map_t
	//
	// Typedef for a concurrent map<> of property strings
	using property_map_t = Concurrency::concurrent_unordered_map<VirtualMachine::Properties, std::string>;

	//-------------------------------------------------------------------------
	// Member Variables

	property_map_t						m_properties;	// Collection of vm properties
	std::shared_ptr<Process>			m_initprocess;	// initial process object
	std::unique_ptr<VmProcessManager>	m_procmgr;		// Process Manager
	std::unique_ptr<VmSystemLog>		m_syslog;		// System Log
	std::unique_ptr<VmFileSystem>		m_vfs;			// Virtual File System

	//
	// PARAMETERS PULLED BACK IN FROM VMSERICEPARAMETERS CLASS
	// NEEDS CLEANUP
	//

	BEGIN_PARAMETER_MAP(VmService)
		// process
		PARAMETER_ENTRY(_T("process.host.32bit"),	process_host_32bit);	// String
		PARAMETER_ENTRY(_T("process.host.64bit"),	process_host_64bit);	// String
		PARAMETER_ENTRY(_T("process.host.timeout"), process_host_timeout);	// DWord
		PARAMETER_ENTRY(_T("systemlog.length"),		systemlog_length);		// DWord
		PARAMETER_ENTRY(_T("vm.initpath"),			vm_initpath);			// String
		PARAMETER_ENTRY(_T("vm.initramfs"),			vm_initramfs);			// String
	END_PARAMETER_MAP()

	// process
	StringParameter			process_host_32bit;
	StringParameter			process_host_64bit;
	DWordParameter			process_host_timeout { 10000 };

	// systemlog
	DWordParameter			systemlog_length { 512 KiB };

	// virtualmachine
	StringParameter			vm_initpath { _T("/sbin/init") };
	StringParameter			vm_initramfs;

};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSERVICE_H_
