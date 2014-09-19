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
#include "syscalls.h"
#include "CompressedStreamReader.h"
#include "CpioArchive.h"
#include "Exception.h"
#include "File.h"
#include "FileSystem.h"
#include "SystemCalls.h"
#include "VmFileSystem.h"
#include "VmProcessManager.h"
#include "VmSystemLog.h"

#include "Host.h"
#include "RootFileSystem.h"

#pragma warning(push, 4)			

//---------------------------------------------------------------------------
// Class VmService

class VmService : public Service<VmService>, private SystemCalls
{
public:

	VmService()=default;

	// CONTROL_HANDLER_MAP
	//
	BEGIN_CONTROL_HANDLER_MAP(VmService)
		CONTROL_HANDLER_ENTRY(SERVICE_CONTROL_STOP, OnStop)
		CONTROL_HANDLER_ENTRY(128, OnUserControl128)
		CONTROL_HANDLER_ENTRY(129, OnUserControl129)
	END_CONTROL_HANDLER_MAP()

	// PARAMETER_MAP
	//
	BEGIN_PARAMETER_MAP(VmService)
		PARAMETER_ENTRY(IDR_PARAM_INITRAMFS, m_initramfs)
		PARAMETER_ENTRY(IDR_PARAM_SYSLOGLENGTH, m_sysloglength)
		PARAMETER_ENTRY(IDR_PARAM_HOSTPROCESS32, m_hostprocess32)
		PARAMETER_ENTRY(IDR_PARAM_HOSTPROCESS64, m_hostprocess64)
		PARAMETER_ENTRY(IDR_PARAM_HOSTPROCESSTIMEOUT, m_hostprocesstimeout)
		PARAMETER_ENTRY(IDR_PARAM_INITPATH, m_initpath)
	END_PARAMETER_MAP()

private:

	VmService(const VmService &rhs)=delete;
	VmService& operator=(const VmService &rhs)=delete;

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
		std::tstring binpath = m_hostprocess32;
		std::unique_ptr<Host> h = Host::Create(binpath.c_str(), m_bindstr32.c_str(), m_hostprocesstimeout);
	}

	// 64-bit host test
	void OnUserControl129(void)
	{
		std::tstring binpath = m_hostprocess64;
		std::unique_ptr<Host> h = Host::Create(binpath.c_str(), m_bindstr64.c_str(), m_hostprocesstimeout);
	}

	// m_initramfs
	//
	// Path to the virtual machine's initramfs blob
	StringParameter m_initramfs;



	std::unique_ptr<VmProcessManager> m_procmgr;

	// m_syslog
	//
	// System log implementation
	std::unique_ptr<VmSystemLog> m_syslog;
	DWordParameter m_sysloglength { 512 KiB };

	std::unique_ptr<VmFileSystem> m_vfs;

	// hosts
	std::tstring m_bindstr32;
	std::tstring m_bindstr64;				// won't be initialized on x86
	StringParameter m_hostprocess32;
	StringParameter m_hostprocess64;
	DWordParameter m_hostprocesstimeout;	// miliseconds
	StringParameter m_initpath;
};

//---------------------------------------------------------------------------

#pragma warning(pop)

#endif	// __VMSERVICE_H_
