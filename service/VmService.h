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

// omg clean these up, can't be using all of these
#include <map>
#include "resource.h"
#include "syscalls.h"
#include "CompressedStreamReader.h"
#include "CpioArchive.h"
#include "Exception.h"
#include "File.h"
#include "FileSystem.h"
#include "SystemCalls.h"
#include "VmFileSystem.h"
#include "VmProcessManager.h"
#include "VmSettings.h"
#include "VmSystemLog.h"

#include "Host.h"
#include "RootFileSystem.h"

#include "VirtualMachine.h"
#include "VmServiceParameters.h"
#include "Process.h"

#pragma warning(push, 4)

//---------------------------------------------------------------------------
// Class VmService
//
// todo: rename me to "VirtualMachine", expose all of the child subsystems
// via properties

class VmService : public Service<VmService>, private SystemCalls, public VmServiceParameters,
	public VirtualMachine
{
public:

	VmService()=default;

	// VirtualMachine Implementation
	//
	virtual const std::unique_ptr<VmFileSystem>&	getFileSystem(void)	const { _ASSERTE(m_vfs);		return m_vfs; }
	virtual const std::unique_ptr<VmSettings>&		getSettings(void)	const { _ASSERTE(m_settings);	return m_settings; }
	virtual const std::unique_ptr<VmSystemLog>&	getSystemLog(void)	const { _ASSERTE(m_syslog);	return m_syslog; }

private:

	VmService(const VmService &rhs)=delete;
	VmService& operator=(const VmService &rhs)=delete;

	// Service<> Control Handler Map
	//
	BEGIN_CONTROL_HANDLER_MAP(VmService)
		CONTROL_HANDLER_ENTRY(SERVICE_CONTROL_STOP, OnStop)
		CONTROL_HANDLER_ENTRY(128, OnUserControl128)
		CONTROL_HANDLER_ENTRY(129, OnUserControl129)
	END_CONTROL_HANDLER_MAP()

	// Service<> Parameter Map --> VmServiceParameters
	//
	// TODO: I want this to be supported in servicelib directly
	virtual void IterateParameters(std::function<void(const svctl::tstring& name, svctl::parameter_base& param)> func)
	{
		// Delegate to the shared VmServiceParameters class
		return VmServiceParameters::IterateParameters(func);
	}

	// LoadInitialFileSystem
	//
	// Loads the initial file system from an initramfs CPIO archive
	void LoadInitialFileSystem(const tchar_t* archivefile);

	// OnStart (Service)
	//
	// Invoked when the service is started
	void OnStart(int argc, LPTSTR* argv);

	// OnStop
	//
	// Invoked when the service is stopped
	void OnStop(void);

	// 32-bit host test
	void OnUserControl128(void)
	{
		FileSystem::HandlePtr p = m_vfs->Open(L"/sbin/init", LINUX_O_RDONLY, 0);
		std::tstring binpath = m_settings->Process.Host32;
		std::unique_ptr<Host> h = Host::TESTME(p, binpath.c_str(), m_bindstr32.c_str(), m_settings->Process.HostTimeout);
	}

	// 64-bit host test
	void OnUserControl129(void)
	{
		//std::tstring binpath = m_settings->Process.Host64;
		//std::unique_ptr<Host> h = Host::Create(binpath.c_str(), m_bindstr64.c_str(), m_hostprocesstimeout);
	}


	// hosts
	std::tstring m_bindstr32;
	std::tstring m_bindstr64;				// won't be initialized on x86

	//-------------------------------------------------------------------------
	// Member Variables

	// Virtual Machine Subsystems
	//
	std::unique_ptr<VmFileSystem>		m_vfs;
	std::unique_ptr<VmProcessManager>	m_procmgr;
	std::unique_ptr<VmSettings>			m_settings;
	std::unique_ptr<VmSystemLog>		m_syslog;
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSERVICE_H_
